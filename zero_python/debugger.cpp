//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <map>
#include <string>
#include <boost/bind.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include "generic/lock.h"
#include "zdk/autocontext.h"
#include "zdk/shared_string_impl.h"
#include "zdk/translation_unit.h"
#include "zdk/zero.h"
#include "breakpoint_callback.h"
#include "debugger.h"
#include "environ.h"
#include "dump_settings.h"
#include "handle_error.h"
#include "locked.h"
#include "macro.h"
#include "marshaller.h"
#include "python_mutex.h"
#include "utility.h"

using namespace std;
using namespace boost;
using namespace python;


void set_temp_breakpoint_(RefPtr<Thread> thread)
{
    if (Debugger* dbg = thread->debugger())
    {
        addr_t pc = thread->program_count();
        dbg->set_temp_breakpoint(get_runnable(thread.get()), pc);
    }
}


static void set_breakpoint_(Debugger* dbg, addr_t addr, PyObject* fun)
{
    if (BreakPointManager* mgr = interface_cast<BreakPointManager*>(dbg))
    {
        if (Thread* thread = dbg->get_thread(DEFAULT_THREAD))
        {
            RefPtr<BreakPointAction> act(new BreakPointCallback(fun));

            mgr->set_breakpoint(get_runnable(thread),
                                BreakPoint::GLOBAL,
                                addr,
                                act.get());
        }
    }
}


static void set_breakpoint(Debugger* dbg, addr_t addr, PyObject* fun)
{
    ThreadMarshaller::instance().send_command(
        bind(set_breakpoint_, dbg, addr, fun), __func__);
}


static void set_breakpoint_(Debugger* dbg, addr_t addr)
{
    if (Thread* thread = dbg->get_thread(DEFAULT_THREAD))
    {
        // set a GLOBAL breakpoint at addr:
        dbg->set_user_breakpoint(get_runnable(thread), addr, true);
    }
}


static void set_user_breakpoint(Debugger* dbg, addr_t addr)
{
    ThreadMarshaller::instance().send_command(
        bind(set_breakpoint_, dbg, addr), __func__);
}


static void
line_to_addr_(Debugger* dbg,
              string file,
              size_t line,
              Debugger::AddrEvents* events)
{
    RefPtr<Thread> thread = dbg->current_thread();
    dbg->line_to_addr(shared_string(file).get(), line, events, thread.get());
}


namespace
{
    class ZDK_LOCAL AddrEvents : public Debugger::AddrEvents
    {
        list addrList_;

        void notify(SymbolTable*, addr_t addr) { addrList_.append(addr); }

    public:
        const list& addr_list() const { return addrList_; }
    };
}


static list line_to_addr(Debugger* dbg, const char* file, size_t line)
{
    AddrEvents events;

    ThreadMarshaller::instance().send_command(
        bind(line_to_addr_, dbg, file, line, &events), __func__);

    return events.addr_list();
}


/**
 * invoke Debugger::quit on the main thread
 */
static void quit(Debugger* dbg)
{
    ThreadMarshaller::instance().send_command(bind(&Debugger::quit, dbg), __func__);
    ThreadMarshaller::shutdown();
}


static list read_settings(Debugger* dbg)
{
    DumpSettings dump;
    try
    {
        if (dbg)
        {
            dbg->read_settings(&dump);
        }
    }
    catch (...)
    {
        python_handle_error();
    }
    return dump.list();
}



/**
 * display asynchronous message (i.e. non-modal)
  */
static void
message_(Debugger* dbg, string msg, Debugger::MessageType type)
{
    MainThreadScope scope;
    dbg->message(msg.c_str(), type, NULL, true);
}


static void
message(Debugger* dbg, const char* msg, Debugger::MessageType type)
{
    ThreadMarshaller::instance().send_command(bind(message_, dbg, msg, type), __func__);
}


static void set_option_(Debugger* dbg, Debugger::Option opt, bool on)
{
    if (on)
    {
        dbg->set_options(dbg->options() | opt);
    }
    else
    {
        dbg->set_options(dbg->options() & ~opt);
    }
}


static void set_option(Debugger* dbg, Debugger::Option opt)
{
    ThreadMarshaller::instance().send_command(
        bind(set_option_, dbg, opt, true), __func__);
}


static void clear_option(Debugger* dbg, Debugger::Option opt)
{
    ThreadMarshaller::instance().send_command(
        bind(set_option_, dbg, opt, false), __func__);
}


/**
 * Execute a debugger shell command on the main thread
 */
static bool command_(Debugger* dbg, string cmd, RefPtr<Thread> thread)
{
    AutoContext ctxt(thread.get(), 0);
    assert(ThreadMarshaller::instance().is_main_thread());
    if (!thread)
    {
        thread = dbg->current_thread();
    }
    bool result = false;
    if (!cmd.empty())
    {
        result = dbg->command(cmd.c_str(), thread.get());
    }
    if (result)
    {
        dbg->resume(true);
    }
    return result;
}


static bool command(Debugger* dbg, const char* cmd, Thread* thread = NULL)
{
    bool ret = false;

    if (cmd)
    {
        // special case, can't execute on the main thread when the debuggee
        // is running, because the debugger is not expecting commands
        if (strcmp(cmd, "stop") == 0)
        {
            if (ThreadMarshaller::instance().is_command_loop_active())
            {
                throw logic_error("Cannot break while command loop is active");
            }
            Lock<Mutex> lock(python_mutex());
            dbg->stop();
        }
        else
        {
            ret = ThreadMarshaller::instance().send_command(
                bind(command_, dbg, cmd, thread),
                cmd);
        }
    }
    return ret;
}


BOOST_PYTHON_FUNCTION_OVERLOADS(cmd_overloads, command, 2, 3)


/**
 * Add a user-defined command (macro)
 */
static void
def_command(Debugger* dbg, const char* name, PyObject* callable, PyObject* dict)
{
    assert(dbg);
    if (CommandCenter* cc = interface_cast<CommandCenter*>(dbg))
    {
        RefPtr<DebuggerCommand> cmd(new Macro(name, callable, dict));
        cc->add_command(cmd.get());
    }
}


static RefPtr<Thread>
get_thread(Debugger* dbg, pid_t lwpid, unsigned long id)
{
    assert(dbg);
    Lock<Mutex> lock(python_mutex());
    return dbg->get_thread(lwpid, id);
}


static RefPtr<Thread> current_thread(Debugger* dbg)
{
    assert(dbg);
    Lock<Mutex> lock(python_mutex());
    return dbg->current_thread();
}


namespace
{
    class ZDK_LOCAL Env : public map<string, string>
    {
        Debugger* dbg_;

    public:
        explicit Env(Debugger* dbg) : dbg_(dbg)
        {
            env_to_map(dbg->environment(), *this);
        }

        ~Env()
        {
            // commit the environment;
            // ideally I should do this only when changes occur
            try
            {
                SArray env;
                map_to_env(*this, env);
                dbg_->set_environment(env.cstrings());
            }
            catch (...)
            {
            }
        }
    };
}


static Env environment(Debugger* debugger)
{
    assert(debugger);
    Lock<Mutex> lock(python_mutex());
    return Env(debugger);
}



static bool
select_thread_(Debugger* debugger, pid_t lwpid, unsigned long tid)
{
    assert(debugger);
    if (Thread* thread = debugger->get_thread(lwpid, tid))
    {
        debugger->set_current_thread(thread);
        return true;
    }
    return false;
}


static void select_thread(Debugger* debugger, pid_t pid, unsigned long tid)
{
    assert(debugger);
    ThreadMarshaller::instance().send_command(
        bind(select_thread_, debugger, pid, tid), __func__);
}


/**
 * Execute target on main debugger thread
 */
static bool exec_(Debugger* dbg, string cmd)
{
    assert(dbg);
    dbg->exec(cmd.c_str());
    dbg->resume(true);
    return true;
}


void load_exec(Debugger* dbg, const char* cmd)
{
    ThreadMarshaller::instance().send_command(bind(exec_, dbg, cmd), __func__);
}


static bool load_core_(Debugger* dbg, string core, RefPtr<SharedString> prog)
{
    dbg->load_core(core.c_str(), prog ? prog->c_str() : NULL);
    dbg->resume(true);
    return true;
}


static void load_core(Debugger* dbg, const char* core, const char* prog)
{
    assert(dbg);
    assert(core);
    RefPtr<SharedString> programFileName;
    if (prog)
    {
        programFileName = shared_string(prog);
    }

    ThreadMarshaller::instance().send_command(
        bind(load_core_, dbg, core, programFileName), __func__);
}


static void detach(Debugger* debugger)
{
    ThreadMarshaller::instance().send_command(
        bind(&Debugger::detach, debugger), __func__);
}


////////////////////////////////////////////////////////////////
// methods for retrieving compilation units
//
/**
 * @note runs on main debugger thread
 */
static void
lookup_unit_by_addr_(Debugger* debugger,
                     addr_t addr,
                     RefPtr<TranslationUnit>& unit)
{
    assert(debugger);
    if (Thread* thread = debugger->get_thread(DEFAULT_THREAD))
    {
        unit = debugger->lookup_unit_by_addr(thread->process(), addr);
    }
}


static RefPtr<TranslationUnit>
lookup_unit_by_addr(Debugger* debugger, addr_t addr)
{
    assert(debugger);
    RefPtr<TranslationUnit> unit;

    // bounce the call onto the main thread, DebugInfo readers
    // may need to read from the debugged program's memory space
    ThreadMarshaller::instance().send_command(
        bind(lookup_unit_by_addr_, debugger, addr, boost::ref(unit)),
        __func__);

    return unit;
}


/**
 * @note runs on main debugger thread
 * @note name is expected to be in "canonical" form
 * @see realpath
 */
static void
lookup_unit_by_name_(Debugger* debugger,
                     string name,
                     RefPtr<TranslationUnit>& unit)
{
    assert(debugger);
    if (Thread* thread = debugger->get_thread(DEFAULT_THREAD))
    {
        unit = debugger->lookup_unit_by_name(thread->process(), name.c_str());
    }
}


static RefPtr<TranslationUnit>
lookup_unit_by_name(Debugger* debugger, const char* name)
{
    assert(debugger);
    RefPtr<TranslationUnit> unit;

    // bounce the call onto the main thread, DebugInfo readers
    // may need to read from the debugged program's memory space
    ThreadMarshaller::instance().send_command(
        bind(lookup_unit_by_name_, debugger, name, boost::ref(unit)),
        __func__);

    return unit;
}
////////////////////////////////////////////////////////////////

/*
static void
always_step_over_(Debugger* debugger, RefPtr<SharedString> path, long line)
{
    assert(debugger);
    debugger->add_step_over(path.get(), line);
}
*/


void export_debugger()
{
    enum_<Debugger::MessageType>("Message")
        .value("Info", Debugger::MSG_INFO)
        .value("Error", Debugger::MSG_ERROR)
        .value("Status", Debugger::MSG_STATUS)
        .value("Help", Debugger::MSG_HELP)
        ;

scope in_Debugger =
    class_<Debugger, bases<>, noncopyable>("Debugger", no_init)
        .def("clear_option", clear_option)
        .def("command", command, cmd_overloads(args("cmd", "thread"),
            "execute a debugger shell command, as if typed at the prompt")
            )
        .def("current_thread", current_thread)
        .def("def_command", def_command)
        .def("detach", detach, "detach from debugged program"
            )
        .add_property("environ", environment)
        .def("load_core", load_core)
        .def("load_exec", load_exec)
        .def("lookup_unit_by_addr", lookup_unit_by_addr,
             "lookup the compilation unit (TranslationUnit) that contains given address"
            )
        .def("lookup_unit_by_name", lookup_unit_by_name,
             "lookup the compilation unit (TranslationUnit) by canonical name"
            )
        .def("get_thread", get_thread)
        .def("line_to_addr", line_to_addr)
        .def("message", message)
        .def("properties", &Debugger::properties,
            return_value_policy<locked<reference_existing_object> >()
            )
        .def("quit", quit)
        .def("set_breakpoint", set_breakpoint,
            "set global breakpoint that calls user defined function",
            return_value_policy<reference_existing_object>()
            )
        .def("set_breakpoint", set_user_breakpoint,
            "set global breakpoint that starts user interactive mode"
            )
        .def("select_thread", select_thread)
        .def("set_option", set_option)
        .def("read_settings", read_settings, "read settings from disk")
        ;

    enum_<Debugger::Option>("Option")
        .value("TraceFork", Debugger::OPT_TRACE_FORK)
        .value("BreakOnSysCalls", Debugger::OPT_BREAK_ON_SYSCALLS)
        .value("TraceSysCalls", Debugger::OPT_TRACE_SYSCALLS)
        .value("Silent", Debugger::OPT_SILENT)
        .value("StartAtMain", Debugger::OPT_START_AT_MAIN)
        ;

    class_<Env>("Environ", init<Debugger*>())
        .def(map_indexing_suite<Env, true>())
        ;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
