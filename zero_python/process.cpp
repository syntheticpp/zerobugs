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
#include <boost/python.hpp>
#include <boost/tokenizer.hpp>
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/get_pointer.h"
#include "zdk/process.h"
#include "zdk/symbol_map.h"
#include "breakpoint_callback.h"
#include "breakpoint_registry.h"
#include "collector.h"
#include "locked.h"
#include "marshaller.h"
#include "process.h"
#include "utility.h"

using namespace std;
using namespace boost;
using namespace python;


/**
 * @return command line of process
 */
static const char* command_line(const Process* proc)
{
    if (SharedString* cmd = proc->command_line())
    {
        return cmd->c_str();
    }
    return "";
}


static void add_shared_object_(RefPtr<Process> proc, string filename)
{
    if (SymbolMap* symbols = proc->symbols())
    {
        symbols->add_module(filename.c_str());
    }
}


static void add_module(Process* proc, const char* filename)
{
    ThreadMarshaller::instance().send_command(
        bind(add_shared_object_, proc, filename), __func__);
}


static void
enum_program_modules(Process* proc, Collector<Module>& collector)
{
    proc->enum_modules(&collector);
}


static python::list enum_modules(Process* proc)
{
    Collector<Module> collector;

    ThreadMarshaller::instance().send_command(
        bind(enum_program_modules, proc, boost::ref(collector)),
        __func__);

    return collector.get();
}


/**
 * @return the environment of the debugged process as a Python dictionary
 */
static dict environment(Process* proc)
{
    typedef char_separator<char> Delim;
    typedef tokenizer<Delim> Tokenizer;

    Delim delim("=");

    dict env;
    const char* const* p = proc->environment();
    for (; p && *p; ++p)
    {
        string tmp(*p);
        Tokenizer tok = Tokenizer(tmp, delim);

        vector<string> v(tok.begin(), tok.end());
        if (v.size() >= 2)
        {
            string val = v[1];
            for (size_t i = 2; i != v.size(); ++i)
            {
                val += '=';
                val += v[i];
            }
            env[ v[0] ] = val;
        }
    }
    return env;
}


////////////////////////////////////////////////////////////////
//
// Per process breakpoints
//
/**
 * Set a breakpoint that calls a Python-defined function
 */
static void set_breakpoint_(RefPtr<Process> proc, addr_t addr, PyObject* fun)
{
    Thread* thread = proc->get_thread(DEFAULT_THREAD);
    if (!thread)
    {
        return;
    }
    if (Debugger* dbg = thread->debugger())
    {
        Runnable* runnable = get_runnable(thread);

        BreakPointManager* mgr = interface_cast<BreakPointManager*>(dbg);
        if (!fun)
        {
            // set GLOBAL breakpoint
            dbg->set_user_breakpoint(runnable, addr, true);
        }
        else if (mgr)
        {
            RefPtr<BreakPointAction> act(new BreakPointCallback(fun));
            mgr->set_breakpoint(runnable, BreakPoint::GLOBAL, addr, act.get());
        }
        if (mgr)
        {
            mgr->enum_breakpoints(&TheBreakPointRegistry::instance(), NULL, addr);
        }
    }
}


static void set_breakpoint(Process* proc, addr_t addr, PyObject* fun = NULL)
{
    ThreadMarshaller::instance().send_command(
        bind(set_breakpoint_, proc, addr, fun), __func__);
}


BOOST_PYTHON_FUNCTION_OVERLOADS(set_breakpoint_overloads, set_breakpoint, 2, 3)


static void
enum_breakpoints_
(
    RefPtr<Process> proc,
    addr_t addr,
    BreakPoint::Type type,
    BreakPointCollector* col
)
{
    if (BreakPointManager* mgr = proc->breakpoint_manager())
    {
        mgr->enum_breakpoints(col, NULL, addr, type);
    }
}


static python::list
enum_breakpoints
(
    Process* proc,
    addr_t addr = 0,
    BreakPoint::Type type = BreakPoint::ANY
)
{
    BreakPointCollector col;

    ThreadMarshaller::instance().send_command(
        bind(enum_breakpoints_, proc, addr, type, &col),
        __func__);
    return col.list();
}


BOOST_PYTHON_FUNCTION_OVERLOADS(breakpoints_overload, enum_breakpoints, 1, 3)


namespace
{
    /**
     * helper for process.line_info(addr) which returns a
     * (file, line, addr-min, addr-max) tuple.
     */
    class LineInfo : public EnumCallback<DebuggerPlugin*>
                           , public EnumCallback2<SharedString*, size_t>
                           , public EnumCallback2<SymbolTable*, addr_t>
    {
        Debugger* dbg_;
        DebugInfoReader* reader_;
        RefPtr<SymbolTable> table_;
        const addr_t addr_;
        addr_t nearest_, last_;
        object* result_;

        void notify(DebuggerPlugin* plugin)
        {
            reader_ = 0;
            if (DebugInfoReader* reader = interface_cast<DebugInfoReader*>(plugin))
            {
                reader_ = reader;
                reader->addr_to_line(table_.get(), addr_, &nearest_, this);
            }
        }

        void notify(SharedString* file, size_t line)
        {
            if (reader_)
            {
                last_ = reader_->next_line_addr(table_.get(), addr_, file, line);
            }
            else if (dbg_)
            {
                dbg_->line_to_addr(file, line, this, dbg_->current_thread());
            }
            *CHKPTR(result_) = python::make_tuple(file->c_str(), line, addr_, last_);
        }

        void notify(SymbolTable*, addr_t addr)
        {
            if (addr > last_)
            {
                last_ = addr;
            }
        }

    public:
        LineInfo(Debugger* dbg, SymbolTable* tbl, addr_t addr, object* res)
            : dbg_(dbg)
            , reader_(0)
            , table_(tbl)
            , addr_(addr)
            , nearest_(0)
            , last_(0)
            , result_(res)
        { assert(dbg_); }
    };
} // namespace


static void line_info_(RefPtr<Process> proc, addr_t addr, object* result)
{
    if (Debugger* debugger = proc->debugger())
    {
        if (SymbolMap* symbols = proc->symbols())
        {
            if (SymbolTable* table = symbols->lookup_table(addr))
            {
                LineInfo callback(debugger, table, addr, result);
                debugger->enum_plugins(&callback);
            }
        }
    }
}


static object line_info(Process* proc, addr_t addr)
{
    object result;
    ThreadMarshaller::instance().send_command(
        bind(line_info_, proc, addr, &result), __func__);
    return result;
}


static void threads_(RefPtr<Process> proc, EnumCallback<Thread*>* callback)
{
    proc->enum_threads(callback);
}


/**
 * get a list of threads for given process
 */
static boost::python::list threads(Process* process)
{
    Collector<Thread> callback;
    ThreadMarshaller::instance().send_command(
        bind(threads_, process, &callback), __func__);

    return callback.get();
}



/**
 * Export the Process interface to python
 */
void export_process()
{
scope in_process =
    class_<Process, bases<>, noncopyable>("Process", no_init)
        .def("add_module", add_module)
        .def("breakpoints", enum_breakpoints,
            breakpoints_overload(args("addr", "type"),
            "return a list of breakpoints for the process")
            )
        .def("command_line", &command_line)
        .def("debugger", &Process::debugger,
            "return the debugger attached to this process",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("environ", environment)
        .def("line_info", line_info)
        .def("origin", &Process::origin,
            "describe how the process was loaded"
            )
        .def("modules", enum_modules,
            "return a list of modules that are loaded by this process"
            )
        .def("name", &Process::name)
        .def("pid", &Process::pid)
        .def("set_breakpoint", set_breakpoint,
            set_breakpoint_overloads(args("addr", "callback"),
                "set global breakpoint at specified address")
            )
        .def("symbols", &Process::symbols,
            "return the symbol map for the process",
            locked<return_value_policy<reference_existing_object> >()
            )

        .def("threads", threads)
        ;

    enum_<ProcessOrigin>("Origin")
        .value("Debugger", ORIGIN_DEBUGGER)
        .value("System", ORIGIN_SYSTEM)
        .value("Core", ORIGIN_CORE)
        ;


    register_ptr_to_python<RefPtr<Process> >();

}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
