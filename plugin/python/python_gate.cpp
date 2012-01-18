//
// $Id: python_gate.cpp 717 2010-10-20 06:17:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <pthread.h>
#include <stdio.h>
#include <fstream>
#include <boost/functional.hpp>
#include <boost/python.hpp>
#include <boost/tokenizer.hpp>
#include "dharma/canonical_path.h"
#include "dharma/config.h"
#include "dharma/environ.h"
#include "dharma/system_error.h"
#include "generic/auto_file.h"
#include "zdk/argv_util.h"
#include "zdk/check_ptr.h"
#include "zero_python/breakpoint.h"
#include "zero_python/breakpoint_registry.h"
#include "zero_python/call.h"
#include "zero_python/class_type.h"
#include "zero_python/data_type.h"
#include "zero_python/debug_event.h"
#include "zero_python/debug_symbol.h"
#include "zero_python/debugger.h"
#include "zero_python/elf_header.h"
#include "zero_python/macro.h"
#include "zero_python/marshaller.h"
#include "zero_python/module.h"
#include "zero_python/process.h"
#include "zero_python/python_embed.h"
#include "zero_python/symbol.h"
#include "zero_python/symbol_map.h"
#include "zero_python/process.h"
#include "zero_python/properties.h"
#include "zero_python/python_embed.h"
#include "zero_python/symbol.h"
#include "zero_python/symbol_map.h"
#include "zero_python/symbol_table.h"
#include "zero_python/stack_trace.h"
#include "zero_python/thread.h"
#include "zero_python/translation_unit.h"
#include "zero_python/type_system.h"
#include "zero_python/update.h"
#include "zero_python/utility.h"
#include "python_gate.h"
#include "filter.h"
#include "updater.h"

using namespace std;
using namespace boost;
using namespace boost::python;


static PythonGate* plugin = 0;
static WeakPtr<Python> interp;

static Debugger* theDebugger = 0; // hack

static Debugger* debugger()
{
    return theDebugger;
}

static void event_loop()
{
    ThreadMarshaller::instance().event_loop();
}

static int event_pipe()
{
    return ThreadMarshaller::instance().event_pipe();
}

static void event_iteration()
{
    ThreadMarshaller::instance().event_iteration(true);
}

static void bypass_builtin_command_interpreter(bool bypass)
{
    if (plugin)
    {
        plugin->set_grab_event(bypass);
    }
}


PythonEmbed& py_interp()
{
    if (RefPtr<Python> python = interp.lock())
    {
        return *python;
    }
    throw runtime_error("Python interpreter not initialized");
}


BOOST_PYTHON_MODULE(zero)
{
    export_breakpoint();
    export_data_type();     // export DataType before ClassType
    export_class_type();    //  since ClassType inherits DataType
    export_debugger();
    export_debug_event();
    export_debug_symbol();
    export_elf_header();
    export_macro();
    export_module();
    export_process();
    export_properties();
    export_symbol();
    export_symbol_map();
    export_symbol_table();
    export_stack_trace();
    export_thread();
    export_translation_unit();
    export_type_system();
    export_update();

    def("bypass_builtin_command_interpreter",
         bypass_builtin_command_interpreter
       );
    def("debugger", debugger, "return current debugger instance",
        return_value_policy<reference_existing_object>()
       );
    def("event_iteration", event_iteration,
        "run one iteration in the event-handling thread");
    def("event_pipe", event_pipe, "return the file descriptor of a pipe"
                                  "that receives notifications when events occur;\n"
                                  "useful when implementing a custom UI. Toolkits "
                                  "such as GTK allow users to define IO handlers.\n");
}


int32_t query_version(int32_t* minor)
{
    if (minor) *minor = ZDK_MINOR;
    return ZDK_MAJOR;
}

/**
 * Advertise the interfaces supported by this plugin
 */
void query_plugin(InterfaceRegistry* registry)
{
    registry->update(DebuggerPlugin::_uuid());
    registry->update(DataFilter::_uuid());
}


/**
 * Create a plugin instance
 */
Plugin* create_plugin(uuidref_t iid)
{
    static UserDataFilter* filter = 0;

    if (uuid_equal(DebuggerPlugin::_uuid(), iid))
    {
        if (!plugin)
        {
            plugin = new PythonGate(interp);

            if (RefPtr<Python> python = interp.lock())
            {
                python->set_context(interface_cast<ZObject*>(plugin));
            }
        }
        return plugin;
    }
    else if (uuid_equal(DataFilter::_uuid(), iid))
    {
        if (!filter)
        {
            filter = new UserDataFilter(interp);
        }
        return filter;
    }
    return 0;
}



PythonGate::PythonGate(WeakPtr<Python>& interp)
    : debugger_(NULL)
    , pyThread_(pthread_t())
    , file_(NULL)
    , argc_(0)
    , argv_(NULL)
    , adapter_(new ZObjectAdapter(this))
    , grabEvent_(false)
    , exitOnExcept_(false)
    , interp_(interp.lock())
{
    if (!interp_)
    {
        interp_.reset(new Python);
        interp = interp_;
    }
    updater_ = new Updater(interp_);
    TheBreakPointRegistry::instance(); // create on main thread
}



PythonGate::~PythonGate() throw()
{
    if (file_)
    {
        fclose(file_);
    }
    if (pyThread_)
    {
        pthread_join(pyThread_, NULL);
    }
    adapter_->clear();
    plugin = NULL;
}



void PythonGate::release()
{
    delete this;
}


/**
 * Check that the interpreter version matches what we
 * compiled against.
 */
static bool check_interp_version(Debugger* debugger)
{
    const char* version = Py_GetVersion();

    if (const char* delim = strchr(version, ' '))
    {
        const string dynVer(version, distance(version, delim));

        if (dynVer != PY_VERSION)
        {
            ostringstream err;
            err << "Python plug-in expects interpreter " << PY_VERSION;
            err << " got: " << dynVer;

            debugger->message(err.str().c_str(), Debugger::MSG_ERROR);
            return false;
        }
    }
    return true;
}

/**
 * Parse command line and other initializing stuff
 */
bool PythonGate::initialize(Debugger* debugger, int* argc, char*** argv)
{
    assert(debugger);

    theDebugger = debugger_ = debugger;
    rcFileName_ = canonical_path("./.zero.py");

    bool checkVersion = true;

    rcFile_.reset(fopen(rcFileName_.c_str(), "r"));
    if (!rcFile_)
    {
        rcFileName_ = env::get_string("ZERO_PLUGIN_PATH") + "/.zero.py";
        rcFile_.reset(fopen(rcFileName_.c_str(), "r"));
    }

BEGIN_ARG_PARSE(argc, argv)
    ON_ARGV("--py-run=", filename_)
    {
        if ((file_ = fopen(filename_.c_str(), "r")) == 0)
        {
            throw SystemError(filename_);
        }
    }
    // same as --py-run, only _exit on exceptions
    ON_ARGV("--py-exec=", filename_)
    {
        exitOnExcept_ = true;
        if ((file_ = fopen(filename_.c_str(), "r")) == 0)
        {
            throw SystemError(filename_);
        }
    }
    ON_ARG("--py-show-cmds")
    {
        ThreadMarshaller::instance().set_debug_commands();
    }
    ON_ARG("--py-show-events")
    {
        ThreadMarshaller::instance().set_debug_events();
    }
    ON_ARG("--py-disable")
    {
        return false;
    }
    ON_ARG("--py-no-version-check")
    {
        checkVersion = false;
    }
END_ARG_PARSE

    if (checkVersion && !check_interp_version(debugger))
    {
        return false;
    }
    argc_ = *argc;
    argv_ = *argv;

    if (PyImport_AppendInittab("zero", initzero) == -1)
    {
        throw runtime_error("Failed to register module __zero__");
    }
    return true;
}



void PythonGate::start()
{
    // instantiate on main thread
    ThreadMarshaller::instance();

    Temporary<bool>  setFlags(exitOnExcept_, true);

    pthread_create(&pyThread_, NULL, interp_thread, this);
    ThreadMarshaller::instance().wait_for_event_activation();
}



void PythonGate::shutdown()
{
    ThreadMarshaller::shutdown();
}


void PythonGate::register_streamable_objects(ObjectFactory*)
{
}



void PythonGate::on_table_init(SymbolTable*)
{
}



static bool table_done(RefPtr<SymbolTable> table)
{
    call_<void>("on_table_done", table);
    return true;
}


void PythonGate::on_table_done(SymbolTable* table)
{
    ThreadMarshaller::instance().send_event(bind(table_done, table), __func__);
}


static bool attached_event(RefPtr<Thread> thread)
{
    if (CHKPTR(thread->process())->enum_threads() == 1)
    {
        call_<void>("on_process", ptr(thread->process()), thread);
    }
    else
    {
        call_<void>("on_thread", thread);
    }
    return true;
}



void PythonGate::on_attach(Thread* thread)
{
    if (thread)
    {
        ThreadMarshaller::instance().send_event(
                bind(attached_event, thread), __func__);
    }
}



static bool detached_event(RefPtr<Thread> thread)
{
    if (!thread)
    {
        call_<void>("on_process_detach", ptr((Process*)0));
    }
    else
    {
        call_<void>("on_thread_detach", thread);

        if (Process* proc = thread->process())
        {
            if (proc->enum_threads() == 0)
            {
                call_<void>("on_process_detach", ptr(proc));
            }
        }
    }
    return true;
}



void PythonGate::on_detach(Thread* thread)
{
    ThreadMarshaller::instance().send_event(
        bind(detached_event, thread), __func__);
}



bool PythonGate::debug_event(RefPtr<DebugEvent> event)
{
    bool ret = call_<bool>("on_event", event);
    return ret;
}



void PythonGate::send_debug_event (
    DebugEvent::Type    type,
    const char*         name,
    Thread*             thread,
    int                 sysCallNum
)
{
    assert (sysCallNum < 0 || type == DebugEvent::SYSCALL_ENTER);

    RefPtr<DebugEvent> event(new DebugEvent(type, thread, sysCallNum));

    bool gtkMode = ThreadMarshaller::instance().is_gtk_mode_active();

    ThreadMarshaller::instance().send_event(
            bind(&PythonGate::debug_event, this, event),
            name,
            gtkMode);
}



void PythonGate::on_syscall(Thread* thread, int32_t num)
{
    send_debug_event(DebugEvent::SYSCALL_ENTER, __func__, CHKPTR(thread), num);
}



bool PythonGate::on_event(Thread* thread, EventType eType)
{
    if (ThreadMarshaller::instance().process_pending_commands())
    {
        return true;
    }

    DebugEvent::Type type = static_cast<DebugEvent::Type>(eType);
    if (thread)
    {
        if ((eType != E_PROMPT) && thread->is_done_stepping())
        {
            if (eType == E_THREAD_RETURN)
            {
                type = DebugEvent::RETURNED;
            }
            else
            {
                type = DebugEvent::DONE_STEPPING;
            }
        }
    }
    send_debug_event(type, __func__, thread);
    Lock<Mutex> lock(mutex_);
    return grabEvent_;
}


void PythonGate::set_grab_event(bool grab)
{
    Lock<Mutex> lock(mutex_);
    grabEvent_ = grab;
}


void PythonGate::on_program_resumed()
{
    // todo: invoke callback in script
}


static void on_brkpnt_ins(RefPtr<Symbol> sym, BreakPoint::Type type)
{
    call_<void>("on_breakpoint_inserted", sym, type);
}


static void on_brkpnt_del(RefPtr<Symbol> sym, BreakPoint::Type type)
{
    call_<void>("on_breakpoint_deleted", sym, type);
}


void PythonGate::on_insert_breakpoint(volatile BreakPoint* bpnt)
{
    assert(bpnt);

    ThreadMarshaller::instance().send_event(
            bind(on_brkpnt_ins, bpnt->symbol(), bpnt->type()),
            __func__);
}


void PythonGate::on_remove_breakpoint(volatile BreakPoint* bpnt)
{
    assert(bpnt);

    ThreadMarshaller::instance().send_event(
            bind(on_brkpnt_del, bpnt->symbol(), bpnt->type()),
            __func__);
}



static bool progress_callback(string what, double percent, bool* cancel)
{
    assert(cancel);
    *cancel = !call_<bool>("on_progress", what, percent);
    return true;
}


bool PythonGate::on_progress(const char* what, double percent, word_t cookie)
{
    bool cancel = false;
    ThreadMarshaller::instance().send_event(
        bind(progress_callback, what, percent, &cancel), __func__);
    return !cancel;
}


const char* PythonGate::description() const
{
    return "PythonGate Plugin";
}


const char* PythonGate::copyright() const
{
    return "";
}


void* PythonGate::interp_thread(void* ptr)
{
    assert(ptr);
    PythonGate* plugin = reinterpret_cast<PythonGate*>(ptr);
    string error;

    try
    {
        plugin->run_interp();
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }
    catch (const error_already_set&)
    {
        error = python_get_error();
    }
    catch (...)
    {
        error = "unknown exception in Python thread\n";
    }
    if (!error.empty())
    {
        if (plugin->exitOnExcept_)
        {
            cerr << error << endl;
            _exit(1);
        }
        ThreadMarshaller::instance().break_command_loop();
        plugin->debugger_->message(error.c_str(), Debugger::MSG_ERROR, NULL, true);
    }
    return NULL;
}


void PythonGate::run_interp()
{
    PySys_SetArgv(argc_, argv_);
    ThreadMarshaller::instance().set_callbacks(this);

    bool haveScript = false;

    // run .zero.py file if detected
    if (rcFile_)
    {
        interp_->run_file(rcFile_.get(), rcFileName_.c_str());
        haveScript = true;
    }
    if (file_)
    {
        interp_->run_file(file_, filename_);
        haveScript = true;
    }
    if (haveScript)
    {
        call_<void>("on_init", ptr(debugger_));
    }
    event_loop();
}


void PythonGate::on_error(const string& msg)
{
    if (!call_<bool>("on_error", msg))
    {
        debugger_->message(msg.c_str(), Debugger::MSG_ERROR, NULL, true);
    }
}


bool PythonGate::on_command_loop_state(bool active)
{
    call_<void>("on_command_loop_state", active);
    return true;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
