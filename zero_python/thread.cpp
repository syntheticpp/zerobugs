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
//
#include <wchar.h>
#include <memory>
#include <boost/python.hpp>
#include <boost/ref.hpp>
#include "zdk/breakpoint_util.h"
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/expr.h"
#include "zdk/get_pointer.h"
#include "zdk/observer_impl.h"
#include "zdk/register.h"
#include "zdk/thread_util.h"
#include "zdk/type_system.h"
#include "zdk/variant_util.h"
#include "zdk/zero.h"
#include "dharma/sigutil.h"
#include "dharma/symbol_util.h"
#include "dharma/type_lookup_helper.h"
#include "breakpoint_callback.h"
#include "breakpoint_registry.h"
#include "debug_sym_wrap.h"
#include "handle_error.h"
#include "locked.h"
#include "marshaller.h"
#include "thread.h"
#include "utility.h"

using namespace std;
using namespace boost;


/**
 * The ZDK exposes Debugger::step, but export it to Python as a
 * member method of Thread (it's more intuitive this way, I think)
 */
static bool step_(RefPtr<Thread> thread, Debugger::StepMode mode)
{
    if (Debugger* dbg = CHKPTR(thread)->debugger())
    {
        if (thread_is_attached(*thread))
        {
            //note: the destination address is 0, meaning the
            //debugger has to automatically figure out next line's
            //address
            dbg->step(thread.get(), mode, thread->program_count(), 0);

            dbg->resume();
            return true;
        }
    }

    return false;
}


static void
step(Thread* thread, Debugger::StepMode mode, int count = 1)
{
    assert(thread);
    if (count == 0)
    {
        return;
    }

    ThreadMarshaller::instance().send_command(bind(step_, thread, mode), __func__);
    while (--count > 0)
    {
        ThreadMarshaller::instance().schedule(bind(step_, thread, mode), __func__);
    }
}
BOOST_PYTHON_FUNCTION_OVERLOADS(step_overloads, step, 2, 3)


/**
 * Fetch the stack trace on the main debugger thread
 */
static void trace_(RefPtr<Thread> thread, size_t maxDepth)
{
    thread->stack_trace(maxDepth);
}


static RefPtr<StackTrace> stack_trace(Thread* thread)
{
    RefPtr<StackTrace> trace;
    if (thread)
    {
        ThreadMarshaller::instance().send_command(
            bind(trace_, thread, INT_MAX), __func__);
        trace = thread->stack_trace();
    }
    return trace;
}



static void frame_regs_(Thread* thread)
{
    thread->program_count();
    thread->frame_pointer();
    thread->stack_pointer();
}


static reg_t program_count(Thread* thread)
{
    if (!thread || thread_finished(*thread))
    {
        return 0;
    }
    ThreadMarshaller::instance().send_command(bind(frame_regs_, thread), __func__);
    return thread->program_count();
}


static reg_t frame_pointer(Thread* thread)
{
    if (!thread || thread_finished(*thread))
    {
        return 0;
    }
    ThreadMarshaller::instance().send_command(bind(frame_regs_, thread), __func__);
    return thread->frame_pointer();
}


static reg_t stack_pointer(Thread* thread)
{
    if (!thread || thread_finished(*thread))
    {
        return 0;
    }
    ThreadMarshaller::instance().send_command(bind(frame_regs_, thread), __func__);
    return thread->stack_pointer();
}


template<typename T>
static void append_symbol(T& container, DebugSymbol* sym, DebugSymbolEvents* events)
{
    container.append(RefPtr<DebugSymbolWrap>(new DebugSymbolWrap(sym, events)));
}

//
// Helper callbacks/observers
//
namespace
{
    enum NumericBase
    {
        OCT = 8,
        DEC = 10,
        HEX = 16
    };


    /**
     * Helper for eval. Collects the results in a Python list
     */
    class ExprEventsObserver : public SubjectImpl<ExprEvents>
    {
        typedef SubjectImpl<ExprEvents> Base;

        int     base_;
        addr_t  addr_;
        mutable Mutex mutex_;
        boost::python::list results_;

        ExprEventsObserver(int base) : base_(base), addr_(0)
        { }

        ExprEventsObserver(const ExprEventsObserver& other)
            : base_(other.base_)
            , addr_(other.addr_)
        {
        }

    public:
        static RefPtr<ExprEventsObserver> create(int base)
        {
            return new ExprEventsObserver(base);
        }

        virtual ~ExprEventsObserver() throw()
        {
        }

        bool on_done(Variant* v, bool*, DebugSymbolEvents* events)
        {
            MainThreadScope scope;
            try
            {
                if (v)
                {
                    if (DebugSymbol* sym = v->debug_symbol())
                    {
                        Lock<Mutex> lock(mutex_);
                        append_symbol(results_, sym, events);
                    }
                    else
                    {
                        ostringstream outs;
                        variant_print(outs, *v);
                        Lock<Mutex> lock(mutex_);
                        results_.append(object(outs.str()));
                    }
                }
            }
            catch (const error_already_set&)
            {
                python_handle_error();
            }
            return true;
        }

        boost::python::list results() const
        {
            Lock<Mutex> lock(mutex_);
            return results_;
        }

        void on_error(const char* errMsg)
        {
            MainThreadScope scope;
            PyErr_SetString(PyExc_RuntimeError, errMsg);
        }

        void on_warning(const char* errMsg)
        {
            Lock<Mutex> lock(mutex_);
            // since Python is a dynamically-typed language, strings
            // and DebugSymbol objects can be mixed in a list;
            // return the warning in the result list

            results_.append(errMsg);
        }

        /**
         * Handle events that occur while interpreting an expression
         */
        bool on_event(Thread* thread, addr_t addr)
        {
            MainThreadScope scope;

            int sig = thread->signal();
            if (addr_ && sig && (addr != addr_))
            {
                ostringstream msg;

                msg << sig_name(thread->signal());
                msg << " occurred at " << hex << showbase << addr;
                msg << " while interpreting expression in lwpid=";
                msg << dec << thread->lwpid();

                if (Debugger* debugger = thread->debugger())
                {
                    debugger->message(msg.str().c_str(), Debugger::MSG_INFO, thread);
                }
                if (sig != SIGTRAP)
                {
                    thread->set_signal(0);
                    Runnable& task = interface_cast<Runnable&>(*thread);
                    task.set_program_count(addr_);
                }
            }
            return true;
        }

        void on_call(addr_t addr, Symbol*) { addr_ = addr; }

        ExprEvents* clone() const
        {
            return new ExprEventsObserver(*this);
        }
    };



    /**
     * Helper for enumerating DebugSymbol instances
     */
    class DebugSymbolObserver : public DebugSymbolEvents
    {
        mutable Mutex mutex_;
        boost::python::list list_;

        bool notify(DebugSymbol* sym)
        {
            Lock<Mutex> lock(mutex_);
            append_symbol(list_, sym, NULL);
            return true;
        }

        bool is_expanding(DebugSymbol*) const
        {
            return false;
        }

        int numeric_base(const DebugSymbol*) const
        {
            return 0;
        }

        void symbol_change(DebugSymbol* newSym, DebugSymbol* old)
        {
        }

        BEGIN_INTERFACE_MAP(DebugSymbolObserver)
        END_INTERFACE_MAP()

    public:
        boost::python::list list() const
        {
            Lock<Mutex> lock(mutex_);
            return list_;
        }

        void set_list(const boost::python::list& list)
        {
            list_ = list;
        }
    };


    /**
     * Stores a collection of variables, so that we can attach
     * them to a stack frame
     */
    class Variables : public ZObjectImpl<>
    {
    public:
        Variables(const boost::python::list& vars, LookupScope scope)
            : vars_(vars), scope_(scope)
        { }

        ~Variables() throw() { }

        const boost::python::list& list() const { return vars_; }

        LookupScope scope() const { return scope_; }

    private:
        boost::python::list vars_;
        LookupScope scope_;
    };

} // namespace


/**
 * Evaluate a C++ expression on the main debugger thread
 */
static bool
eval_(RefPtr<Thread> thread,
      string expr, // purposely by value
      RefPtr<ExprEvents> events,
      int base)
{
    bool resume = false;
    if (Debugger* debugger = thread->debugger())
    {
        if (!debugger->evaluate(expr.c_str(), thread.get(), 0, events.get(), base))
        {
            debugger->resume();
            resume = true;
        }
    }
    return resume;
}


/**
 * Bounce a call to Debugger::evaluate on to the main debugger thread.
 * Allows the client to specify a numeric base for displaying the results.
 */
static boost::python::list
eval_base(Thread* thread, const char* expr, NumericBase numericBase)
{
    RefPtr<ExprEventsObserver> obs = ExprEventsObserver::create(numericBase);

    ThreadMarshaller::instance().send_command(
        bind(eval_, thread, expr, obs, numericBase), __func__);
    ThreadMarshaller::instance().wait_for_main_thread();
    return obs->results();
}


/**
 * Bounce a call to Debugger::evaluate on to the main debugger thread.
 * Uses decimal (numeric base ten) for representing the results.
 */
static boost::python::list eval_decimal(Thread* thread, const char* expr)
{
    return eval_base(thread, expr, DEC);
}


/**
 * Enumerate the variables that match the given name and scope,
 * in the context of the currently selected frame of the given thread.
 * The function runs on the main thread.  If name is empty, all variables
 * in the specified scope are returned.
 */
static void vars_(
    RefPtr<Thread> thread,
    string name,
    DebugSymbolObserver* events,
    LookupScope scope)
{
    RefPtr<Frame> frame = thread_current_frame(thread.get());
    if (frame)
    {
        RefPtr<Variables> vars =
            interface_cast<Variables*>(frame->get_user_object("_vars"));

        if (vars && vars->scope() == scope)
        {
            events->set_list(vars->list());
            return;
        }
    }

    if (Debugger* dbg = thread->debugger())
    {
        dbg->enum_variables(thread.get(), name.c_str(), 0, events, scope);

        if (frame)
        {
            RefPtr<Variables> vars(new Variables(events->list(), scope));
            frame->set_user_object("_vars", vars.get());
        }
    }
}


namespace
{
    /**
     * Helper: selects a stack frame in the current scope,
     * and then restores the current selection using RAII
     */
    class FrameScope : noncopyable
    {
        RefPtr<StackTrace> trace_;
        size_t current_;

    public:
        FrameScope(StackTrace* trace, size_t frame)
            : trace_(trace), current_(static_cast<size_t>(-1))
        {
            current_ = CHKPTR(trace->selection())->index();
            trace->select_frame(frame);
        }
        ~FrameScope()
        {
            assert(current_ != static_cast<size_t>(-1));
            trace_->select_frame(current_);
        }
    };
}


/**
 * Enumerate parameters in the specified stack frame
 */
static void param_(
    RefPtr<Thread> thread,
    RefPtr<Frame> frame,
    string name,        // filter by name, empty matches everything
    DebugSymbolEvents* events)
{
    if (Debugger* dbg = thread->debugger())
    {
        FrameScope frameScope(CHKPTR(thread->stack_trace()), frame->index());

        dbg->enum_variables(thread.get(), name.c_str(),
            frame->function(), events, LOOKUP_PARAM);
    }
}


static boost::python::list
variables(Thread* thread, LookupScope s, const char* name = "")
{
    DebugSymbolObserver obs;

    ThreadMarshaller::instance().send_command(
        bind(vars_, thread, name, &obs, s), __func__);
    return obs.list();
}


BOOST_PYTHON_FUNCTION_OVERLOADS(var_overloads, variables, 2, 3)


static boost::python::list
param(Thread* thread, Frame* frame, const char* name = "")
{
    DebugSymbolObserver obs;

    ThreadMarshaller::instance().send_command(
        bind(param_, thread, frame, name, &obs), __func__);
    return obs.list();
}


BOOST_PYTHON_FUNCTION_OVERLOADS(param_overloads, param, 2, 3)

////////////////////////////////////////////////////////////////
//
// Per thread breakpoints
//
/**
 * Set breakpoint that calls back python-defined function
 */
static void
set_breakpoint_(RefPtr<Thread> thread, addr_t addr, PyObject* fun)
{
    if (Debugger* dbg = thread->debugger())
    {
        if (BreakPointManager* mgr = interface_cast<BreakPointManager*>(dbg))
        {
            Runnable* runnable = get_runnable(thread.get());

            RefPtr<BreakPointAction> act(new BreakPointCallback(fun));
            mgr->set_breakpoint(runnable, BreakPoint::PER_THREAD, addr, act.get());
            mgr->enum_breakpoints(&TheBreakPointRegistry::instance(), thread.get(), addr);
        }
    }
}


static void set_breakpoint(Thread* thread, addr_t addr, PyObject* fun)
{
    ThreadMarshaller::instance().send_command(
        bind(set_breakpoint_, thread, addr, fun), __func__);
}


/**
 * Set breakpoint that begins interactive mode (waits for a user to
 * give a command)
 */
static void set_breakpoint_(RefPtr<Thread> thread, addr_t addr)
{
    if (Debugger* dbg = thread->debugger())
    {
        Runnable* runnable = get_runnable(thread.get());

        dbg->set_user_breakpoint(runnable, addr, true, true);
        if (BreakPointManager* mgr = interface_cast<BreakPointManager*>(dbg))
        {
            mgr->enum_breakpoints(&TheBreakPointRegistry::instance(), thread.get(), addr);
        }
    }
}


static void set_user_breakpoint(Thread* thread, addr_t addr)
{
    ThreadMarshaller::instance().send_command(
        bind(set_breakpoint_, thread, addr), __func__);
}



static word_t get_result(Thread* thread)
{
    assert(thread);
    return thread->result();
}


static void
enum_breakpoints_(RefPtr<Thread> thread,
                  addr_t addr,
                  BreakPoint::Type type,
                  BreakPointCollector* col)
{
    if (Debugger* dbg = thread->debugger())
    {
        if (BreakPointManager* mgr = interface_cast<BreakPointManager*>(dbg))
        {
            mgr->enum_breakpoints(col, thread.get(), addr, type);
        }
    }
}


static boost::python::list
enum_breakpoints
(
    Thread* thread,
    addr_t addr = 0,
    BreakPoint::Type type = BreakPoint::ANY
)
{
    BreakPointCollector col;

    ThreadMarshaller::instance().send_command(
        bind(enum_breakpoints_, thread, addr, type, &col),
        __func__);
    return col.list();
}


BOOST_PYTHON_FUNCTION_OVERLOADS(breakpoints_overload, enum_breakpoints, 1, 3)



namespace
{
    /**
     * Helper for collecting the registers and returning them
     * as a Python dictionary
     */
    class RegisterCallback : public EnumCallback<Register*>
    {
    private:
        mutable Mutex mutex_;
        dict dict_;

    public:
        boost::python::dict dictionary() const
        {
            Lock<Mutex> lock(mutex_);
            return dict_;
        }

        void notify(Register* reg)
        {
            if (Variant* val = reg->value())
            {
                Lock<Mutex> lock(mutex_);
                if (::is_float(*val))
                {
                    dict_[reg->name()] = val->long_double();
                }
                else
                {
                    dict_[reg->name()] = val->uint64();
                }
            }
        }
    };
}


static void regs_(Thread* thread, EnumCallback<Register*>* callback)
{
    thread->enum_cpu_regs(callback);
}


/**
 * @return the CPU registers for this thread, in a Dictionary
 */
static dict regs(Thread* thread, Frame* frame = NULL)
{
    auto_ptr<FrameScope> frameScope;
    if (frame)
    {
        // assume that if we have a frame there's also a stack trace
        // and we don't need a roundtrip to the main debugger thread
        StackTrace* trace = CHKPTR(thread->stack_trace());
        frameScope.reset(new FrameScope(trace, frame->index()));
    }
    RegisterCallback callback;

    ThreadMarshaller::instance().send_command(bind(regs_, thread, &callback), __func__);
    return callback.dictionary();
}


BOOST_PYTHON_FUNCTION_OVERLOADS(regs_overloads, regs, 1, 2)


static bool finished(Thread* thread)
{
    return thread_finished(*thread);
}



namespace
{
    class FunRangeHelper : public EnumCallback<DebuggerPlugin*>
    {
        RefPtr<Thread> thread_;
        addr_t addr_, begin_, end_;

    public:
        FunRangeHelper(const RefPtr<Thread>& thread, addr_t addr)
            : thread_(thread), addr_(addr), begin_(0), end_(0)
        { }

        void notify(DebuggerPlugin* plugin)
        {
            if (begin_ && end_)
            {
                return;
            }
            if (DebugInfoReader* reader = interface_cast<DebugInfoReader*>(plugin))
            {
                reader->get_fun_range(thread_.get(), addr_, &begin_, &end_);
            }
        }

        addr_t begin() const { return begin_; }
        addr_t end() const { return end_; }
    };
}


static void fun_range_(RefPtr<Thread> thread, addr_t addr, object* result)
{
    if (Debugger* dbg = thread->debugger())
    {
        FunRangeHelper helper(thread, addr);
        dbg->enum_plugins(&helper);

        *result = boost::python::make_tuple(helper.begin(), helper.end());
    }
}


static object fun_range(Thread* thread, addr_t addr)
{
    object range;

    ThreadMarshaller::instance().send_command(
        bind(fun_range_, thread, addr, &range),
        __func__);

    return range;
}


//
// Disassemble
//
enum DisasmFormat
{
    DISASM_ONLY,
    DISASM_WITH_SOURCE
};

namespace
{
    class DisasmOutput : public Disassembler::OutputCallback
    {
        boost::python::list list_;
        Mutex mutex_;

        bool notify(addr_t addr, const char* text, size_t len)
        {
            if (Lock<Mutex>(mutex_))
            {
                list_.append(boost::python::make_tuple(addr, text, len));
            }
            return true;
        }

        bool tabstops(size_t* first, size_t* second) const
        {
            if (first)
            {
                *first = sizeof (addr_t) * 2 + 2;
            }
            if (second)
            {
                *second = sizeof (addr_t) * 2 + 32;
            }
            return true;
        }

    public:
        const boost::python::list& listing() const
        {
            return list_;
        }
    };
}


static void disassemble_(
    RefPtr<Thread> thread,
    RefPtr<Symbol> symbol, // start symbol
    size_t howManyBytes,
    DisasmFormat format,
    Disassembler::OutputCallback* callback)
{
    assert(thread);
    assert(symbol);

    Debugger* debugger = thread->debugger();

    const uint8_t* membuf = NULL;
    vector<uint8_t> buf;

    Disassembler* disasm = interface_cast<Disassembler*>(debugger);
    if (disasm && !disasm->uses_own_buffer())
    {
        size_t nwords = (howManyBytes + sizeof(word_t) - 1) / sizeof (word_t);
        buf.resize(nwords * sizeof(word_t));

        thread->read_code(symbol->addr(), (word_t*)&buf[0], nwords, &nwords);
        membuf = &buf[0];
        howManyBytes = nwords * sizeof(word_t);
    }
    if (debugger)
    {
        debugger->disassemble(  thread.get(),
                                symbol.get(),
                                howManyBytes,
                                format,
                                membuf,
                                callback);
    }
}


static boost::python::list disassemble(Thread* thread,
                                       Symbol* start,
                                       size_t howManyBytes,
                                       DisasmFormat fmt)
{
    assert(thread);
    DisasmOutput out;

    if (start)
    {
        ThreadMarshaller::instance().send_command(
            bind(disassemble_, thread, start, howManyBytes, fmt, &out),
            __func__);
    }
    return out.listing();
}


/**
 * Lookup a type by name
 */
static void
lookup_type_(RefPtr<Thread> thread,
             string name,
             LookupScope scope,
             RefPtr<DataType>& type)
{
    if (Debugger* debugger = thread->debugger())
    {
        TypeLookupHelper helper(*thread, name, 0, scope);

        debugger->enum_plugins(&helper);
        type = helper.type();
    }
}


static RefPtr<DataType>
lookup_type(Thread* thread, const char* name, LookupScope scope = LOOKUP_MODULE)
{
    RefPtr<DataType> type;

    ThreadMarshaller::instance().send_command(
            bind(lookup_type_, thread, name, scope, boost::ref(type)),
            __func__);
    return type;
}


static void
set_traceable_(RefPtr<Thread> thread, bool traceable)
{
    thread->set_traceable(traceable);
}


static void
set_traceable(Thread* thread, bool traceable)
{
    ThreadMarshaller::instance().send_command(
            bind(set_traceable_, thread, traceable),
            __func__);
}


BOOST_PYTHON_FUNCTION_OVERLOADS(lookup_type_overload, lookup_type, 2, 3)



template<int CharSize>
static void
read_ucs_(RefPtr<Thread> thread,
          addr_t addr,
          size_t len,
          string* ret)

{
    const size_t nwords =
        (len * CharSize + sizeof(word_t) - 1) / sizeof(word_t);

    vector<word_t> buf(nwords);
    thread->read_data(addr, &buf[0], nwords);

    union
    {
        uint8_t b[CharSize];
        wchar_t w;
    } uc;

    wstring wstr;
    for (size_t i = 0; i != len; ++i)
    {
        uc.w = 0;
        memcpy(&uc.b[0], (char*)(&buf[0]) + i * CharSize, CharSize);
        wstr += uc.w;
    }

    assert(wstr.size() == len);

    vector<char> str(len * 4 + 2);
    str[0] = '\"';
    mbstate_t msb = { 0 };

    const wchar_t* w = wstr.c_str();

    if (wcsrtombs(&str[1], &w, str.size() - 2, &msb) != size_t(-1))
    {
        *ret = &str[0];
        *ret += '\"';
    }
}


static string
read_as_ucs2(Thread* thread, addr_t addr, size_t len)
{
    string ret;

    ThreadMarshaller::instance().send_command(
        bind(read_ucs_<2>, thread, addr, len, &ret), __func__);

    return ret;
}


static string
read_as_ucs4(Thread* thread, addr_t addr, size_t len)
{
    string ret;

    ThreadMarshaller::instance().send_command(
        bind(read_ucs_<4>, thread, addr, len, &ret), __func__);
    return ret;
}


/**
 * Export the thread interface to Python
 */
void export_thread()
{
    enum_<NumericBase>("NumericBase") // for eval
        .value("dec", DEC)
        .value("oct", OCT)
        .value("hex", HEX)
        ;

    enum_<DisasmFormat>("Disasm")
        .value("NoSource", DISASM_ONLY)
        .value("WithSource", DISASM_WITH_SOURCE)
        ;

    enum_<Debugger::StepMode>("Step")
        .value("Instruction", Debugger::STEP_INSTRUCTION)
        .value("OverInstruction", Debugger::STEP_OVER_INSTRUCTION)
        .value("Statement", Debugger::STEP_SOURCE_LINE)
        .value("OverStatement", Debugger::STEP_OVER_SOURCE_LINE)
        .value("Return", Debugger::STEP_RETURN)
        ;

    enum_<LookupScope>("Scope")
        .value("Local", LOOKUP_LOCAL)
        .value("All", LOOKUP_ALL)
        .value("Module", LOOKUP_MODULE)
        .value("Param", LOOKUP_PARAM)
        .value("Unit", LOOKUP_UNIT)
        ;

    class_<Thread, bases<>, boost::noncopyable>("Thread", no_init)
        .def("breakpoints", enum_breakpoints,
            breakpoints_overload(args("args", "type"),
            "return a list of breakpoints per thread")
            )

        .def("debugger", &Thread::debugger,
            "return the debugger instance attached to this thread",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("disassemble", disassemble,
            "disassemble starting at the specified symbol\n"
            "returns 3-tuples of address, text, length of opcode\n"
            )
        .def("eval", eval_base,
            "evaluate C++ expression in the context of the given thread"
            )
        .def("eval", eval_decimal,
            "evaluate C++ expression in the context of the given thread"
            )
        .def("finished", finished, "return true if the thread has finished",
            locked<>()
            )
        .def("frame_pointer", frame_pointer,
            "return the current frame pointer"
            )
        .def("fun_range", fun_range)
        .def("id", &Thread::thread_id,
            "return pthread ID", locked<>()
            )

        .def("is_32_bit", &Thread::is_32_bit)
        .def("is_done_stepping", &Thread::is_done_stepping,
             "return true if finished stepping", locked<>()
            )
        .def("is_execed", &Thread::is_execed)
        .def("is_forked", &Thread::is_forked)
        .def("is_live", &Thread::is_live)
        .def("is_traceable", &Thread::is_traceable)
        .def("gid", &Thread::gid)
        .def("group_id", &Thread::gid)
        .def("lwpid", &Thread::lwpid,
            "return the ID of the lightweight process associated with this thread",
            locked<>()
            )
        .def("lookup_type", lookup_type, lookup_type_overload(args("name","scope"),
             "lookup a data type by name, in the virtual space of this thread")
            )
        .def("parent_id", &Thread::ppid)
        .def("ppid", &Thread::ppid)
        .def("process", &Thread::process,
            "return the process the thread belongs to",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("program_count", program_count,
            "return the current program counter"
            )
        .def("read_as_ucs2", read_as_ucs2)
        .def("read_as_ucs4", read_as_ucs4)
        .def("regs", regs, regs_overloads(args("frame"),
            "return CPU regs for a given thread and (optional) stack frame")
            )
        .def("result", get_result, "return result of last operation")
        .def("set_breakpoint", set_breakpoint)
        .def("set_breakpoint", set_user_breakpoint)
        .def("symbols", &Thread::symbols,
            "return the symbol map for the process",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("set_traceable", set_traceable)
        .def("stack_pointer", stack_pointer,
            "return the current stack pointer"
            )
        .def("stack_trace", stack_trace,
            "return the current stack trace"
            )
        .def("step", &step, step_overloads(args("mode", "count"),
            "step through the debugged program")
            )
        .def("variables", &variables, var_overloads(args("scope","name"),
            "return the variables that match the given name and scope,\n"
            "in the context of the currently selected frame of the given thread;\n"
            "an empty name string matches everything")
            )
        .def("param", param,
            param_overloads(args("frame", "name"),
                "return a list of parameters in specified frame")
            )
        ;

    register_ptr_to_python<RefPtr<Thread> >();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
