//
// $Id: debugger_engine.cpp 729 2010-10-31 07:00:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#ifdef HAVE_SYS_PARAM_H
 #include <sys/param.h>
#endif
#ifdef HAVE_SYS_SYSCALL_H
 #include <sys/syscall.h>
#endif
#include <functional>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/format.hpp>
#include "zdk/argv_util.h"
#include "zdk/breakpoint_util.h"
#include "zdk/debug_sym.h"
#include "zdk/disasm.h"
#include "zdk/expr.h"
#include "zdk/check_ptr.h"
#include "zdk/priority.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/type_system_util.h"
#include "zdk/variant.h"
#include "zdk/zobject_impl.h"
#include "zdk/zobject_scope.h"
#include "generic/auto_file.h"
#include "generic/lock.h"
#include "dharma/config.h"
#include "dharma/environ.h"
#include "dharma/exec.h"
#include "dharma/exec_arg.h"
#include "dharma/fstream.h"
#include "dharma/path.h"
#include "dharma/process_name.h"
#include "dharma/sigutil.h"
#include "dharma/symbol_util.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"
#include "dharma/switchable_action.h"
#if HAVE_ELF
 #include "elfz/public/binary.h"
 #include "elfz/public/link.h"
 #include "elfz/public/program_header.h"
#endif
#include "interp/context_impl.h"
#include "interp/interp.h"
#include "symbolz/private/link_data.h"
#include "symbolz/public/symbol_table_events.h"
#include "typez/public/debug_symbol.h"
#include "typez/public/lookup_child.h"
#include "typez/public/type_system.h"
#include "typez/public/util.h"
#include "breakpoint_mgr.h"
#include "debugger_engine.h"
#include "expr_events_wrapper.h"
#include "expr_observer.h"
#include "query_symbols.h"  // DebugSymbolHelpers
#include "process.h"
#include "ptrace.h"
#include "step_over_mgr.h"
#include "symbol_events.h"
#include "thread.h"
#include "unwind_stack_frame.h"
#include "update_impl.h"
#include "watchpoint.h"

#ifndef NSIG
 #define NSIG _NSIG
#endif

using namespace std;
using namespace boost;
using namespace eventlog;


static const char copyrightText[] = "Copyright (C) 2004 - 2012 Cristian L. Vlasceanu.\n";


/**
 * Attach a string describing the last event
 */
template<typename T>
static void set_event_description
(
    Thread& thread,
    addr_t addr,
    const T& event,
    const char* detail = 0
)
{
    boost::format fmt("%s at: %p");
    string str = (fmt % event % (void*)addr).str();

    if (detail)
    {
        str += " ";
        str += detail;
    }
    thread_set_event_description(thread, str.c_str());
}


static bool quick_shutdown()
{
    static const bool quick = env::get_bool("ZERO_QUICK_SHUTDOWN");
    return quick;
}


static bool inline support_d_language(Debugger&, Thread& thread)
{
    // explicitly disabled by user?
    if (!env::get_bool("ZERO_D_SUPPORT", true))
    {
        return false;
    }
    if (env::get_bool("ZERO_D_SUPPORT", false))
    {
        return true;
    }
    SymbolEnum syms;

    CHKPTR(thread.symbols())->enum_symbols(
        "_Dmain",
        &syms,
        SymbolTable::LKUP_ISMANGLED);

    if (!syms.empty())
    {
        setenv("ZERO_D_SUPPORT", "true", 1);
        cplus_unmangle_d(); // activate D demangler
        return true;
    }
    return false;
}


namespace
{
/**
 * Implementation of the BreakPoint::Action interface;
 * wraps a DebuggerEngine method. Useful for setting
 * breakpoints that call back into the DebuggerEngine.
 */
class ZDK_LOCAL EngineAction : public SwitchableAction
{
public:
    //
    // prototype for calling back a debugger method
    //
    typedef void (DebuggerEngine::*Fun)(
        RefPtr<Thread>, RefPtr<BreakPoint>);

BEGIN_INTERFACE_MAP(EngineAction)
    INTERFACE_ENTRY_INHERIT(SwitchableAction)
END_INTERFACE_MAP()

    EngineAction
    (
        DebuggerEngine& engine,
        const string&   name,
        Fun             fun,
        bool            keep = true,
        word_t          cookie = 0
    )
    : SwitchableAction(name, keep, cookie), fun_(fun)
    { }

    ~EngineAction() throw() {}

private:
    void execute_impl
    (
        Debugger&   debugger,
        Thread*     thread,
        BreakPoint* breakpoint
    )
    {
        DebuggerEngine& engine = interface_cast<DebuggerEngine&>(debugger);
        (engine.*fun_)(thread, breakpoint);
    }

private:
    Fun fun_;
};

} // namespace


////////////////////////////////////////////////////////////////
DebuggerEngine::EventSchedule::EventSchedule(EventSchedule*& self)
    : self_(self)
    , eventType_(E_DEFAULT)
    , interactive_(false)
    , step_(false)
    , exprEventsHandled_(false)
{
    assert(self_ == 0);
    self_ = this;
}


////////////////////////////////////////////////////////////////
DebuggerEngine::EventSchedule::~EventSchedule()
{
    self_ = 0;
}


////////////////////////////////////////////////////////////////
DebuggerEngine::DebuggerEngine()
    : PluginManager(ENGINE_MAJOR, ENGINE_MINOR)
    , sigActions_(NSIG)
// delay until first thread attached
//  , exprObserver_(ExprObserver::create())
    , stepOverMgr_(new StepOverManager(this))
    , argc_(0)
    , argv_(0)
    , sched_(NULL)
    , pluginsInitialized_(false)
    , quitCommandPending_(false)
    , shutdownPending_(false)
    , updateMgr_(new UpdateManager(this))
{
#if (__GNUC__ >= 3) && (FAST_ALLOC_ENABLED)
    //
    // some constructs in boost/pool require GCC 3 or above
    //
    gms::PoolManager::getInstance();
#endif
    uint64_t opts = options();

    if (!env::get_bool("ZERO_HARDWARE_BREAKPOINTS", true))
    {
        opts &= ~OPT_HARDWARE_BREAKPOINTS;
    }
    if (env::get_bool("ZERO_START_MAIN", false))
    {
        opts |= OPT_START_AT_MAIN;
    }
    set_options(opts);
}


////////////////////////////////////////////////////////////////
DebuggerEngine::~DebuggerEngine() throw()
{
    try
    {
        // we need to clear the properties before the plug-ins
        // are unloaded, just in case one of the plug-ins has
        // added a sub-object to the properties
        clear_properties();

        shutdown();
        unload_unreferenced();
    }
    catch (const std::exception& e)
    {
#ifdef DEBUG
        clog << "~DebuggerEngine: " << e.what() << endl;
#endif
    }
}


////////////////////////////////////////////////////////////////
bool
DebuggerEngine::initialize(int argc, char* argv[], ExecArg& args)
{
    int verbosity = 0, i = 1;

    if (argv)
    {
        setenv("ZERO_EXE", argv[0], 1);
    }
    for (bool passThru = false; i < argc; )
    {
        if (passThru)
        {
            args.push_back(argv[i]);
            argv_shift(&argc, &argv[i], i);
        }
        else if (*argv[i] == '-')
        {
            if (argv_match(&argc, &argv, i, "-v", "--verbose"))
            {
                ++verbosity;
            }
            else if (argv_match(&argc, &argv, i, "-h", "--help"))
            {
                print_help(cout);
                return false;
            }
            else if (argv_match(&argc, &argv, i, "--main"))
            {
                set_options(options() | OPT_START_AT_MAIN);
            }
            else if (argv_match(&argc, &argv, i, "--no-main"))
            {
                set_options(options() & ~OPT_START_AT_MAIN);
            }
            else if (argv_match(&argc, &argv, i, "--no-hw-bp"))
            {
                set_use_hardware_breakpoints(false);
            }
            else if (argv_match(&argc, &argv, i, "--trace-fork"))
            {
                set_trace_fork(true);
            }
            else if (argv_match(&argc, &argv, i, "--no-trace-fork"))
            {
                set_trace_fork(false);
            }
            else if (argv_match(&argc, &argv, i, "--fork"))
            {
                set_initial_thread_fork(true);
            }
            else if (argv_match(&argc, &argv, i, "--spawn-on-fork"))
            {
                set_options(options() | OPT_SPAWN_ON_FORK);
            }
            else if (argv_match(&argc, &argv, i, "--no-banner"))
            {
                set_options(options() | OPT_NO_BANNER);
            }
            else if (argv_match(&argc, &argv, i, "--syscall-break"))
            {
                set_options(options() | OPT_BREAK_ON_SYSCALLS);
            }
            else if (argv_match(&argc, &argv, i, "--syscall-trace"))
            {
                set_options(options() | OPT_TRACE_SYSCALLS);
            }
            else if (argv_match(&argc, &argv, i, "--break-on-throw"))
            {
                set_options(options() | OPT_BREAK_ON_THROW);
            }
            else if (argv_match(&argc, &argv, i, "--"))
            {
                passThru = true;
            }
            else
            {
                ++i;
            }
        }
        else
        {
            if (i <= 2 && isdigit(argv[i][0]))
            {
                args.push_back(string("--pid=") + argv[i]);
            }
            else
            {
                args.push_back(argv[i]);
            }
            ++i;
        }
    }
    set_verbose(verbosity);

    if (!use_hardware_breakpoints())
    {
        clog << "Hardware breakpoints disabled\n";
    }
    argc_ = argc;
    argv_ = argv;

    return true;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::detach()
{
    exprObserver_.reset();

    // capture state before breakpoints are disabled
    take_snapshot();

    detach_targets();
    assert(!is_attached()); // post-condition

    symbolEvents_.reset();
    print_counted_objects(__PRETTY_FUNCTION__);
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::shutdown_plugins()
{
    PluginList plugins(plugins_);
    for (PluginList::iterator i = plugins.begin();
         i != plugins.end();
         ++i)
    {
        (*i)->shutdown();
    }
    dbgout(0) << __func__ << ": completed." << endl;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::detach_targets()
{
    if (shutdownPending_)
    {
        return;
    }

    // any live threads still attached?
    if (is_attached() && !has_corefile())
    {
        if (breakPointMgr_.get())
        {
            enable_breakpoints(false, BreakPoint::ANY);
        }
    }
    dbgout(0) << "notifying plugins on_detach" << endl;

    // Notify all plug-in components that we have detached.

    // Yes, this was ugly to type and resists comprehension, to
    // quote Scott Meyers, but hey, if you can't read it, Nirvana
    // is many, many lifetimes ahead for you :)
    for_each_plugin(
        compose1(
            bind2nd(mem_fun(&DebuggerPlugin::on_detach), (Thread*)0),
            mem_fun_ref(&ImportPtr<DebuggerPlugin>::get)));

    vector<RefPtr<BreakPointAction> >(NSIG).swap(sigActions_);

    save_properties(); // save the current snapshot
    breakPointMgr_.reset();

    // the following has the side-effect of preventing a
    // history snapshot to be taken after this point; the
    // breakpoint manager has been reset, and trying to
    // persist the state will result in all breakpoints being
    // "forgotten":
    clear_properties();

    DebuggerBase::detach();
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::quit()
{
    if (!pluginsInitialized_)
    {
        quitCommandPending_ = true;
    }
    else
    {
        shutdown();
        DebuggerBase::quit();
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::shutdown()
{
    if (!shutdownPending_)
    {
        try
        {
            if (quick_shutdown())
            {
                take_snapshot();
                detach_targets();
                _exit(0);
            }
            shutdown_plugins();

            disasm_.reset();
            dataFilter_.reset();
            frameHandlers_.clear();
            plugins_.clear();

            detach();
            shutdownPending_ = true;
        }
        catch (const std::exception& e)
        {
            cout << __func__ << ": " << e.what() << endl;
            _exit(1);
        }
        dbgout(0) << __func__ << ": complete" << endl;
    }
}



////////////////////////////////////////////////////////////////
void DebuggerEngine::on_attach(Thread& thread)
{
    properties(); // ensure that settings are up to date

    dbgout(0) << "on_attach: " << thread.lwpid() << endl;
    bool firstThread = false;

    // fetch the thread ID, in case it has not
    // been initialized yet
    thread.thread_id();

#ifdef DEBUG
    if (verbose())
    {
        print_breakpoints(clog);
    }
#endif // DEBUG

    if (!breakPointMgr_)
    {
        // create a new, clean-state breakpoint manager
        breakPointMgr_.reset(
            new BreakPointManagerGroup(
                CHKPTR(thread.process()),
                verbose(),
                use_hardware_breakpoints(),
                &DebuggerEngine::on_insert,
                &DebuggerEngine::on_remove));

        firstThread = true;
        exprObserver_ = ExprObserver::create();
    }
    // call the base impl, to set the ptrace options
    DebuggerBase::on_attach(thread);

    assert(breakPointMgr_.get()); // on_attach post-condition

    if (thread.is_execed())
    {
        CHKPTR(breakPointMgr_)->on_exec(thread);
    }
    else
    {
        // handle fork()-ed and clone()-d threads
        CHKPTR(breakPointMgr_)->on_thread_created(thread);
    }
    // Notify all plug-ins that we have attached to a thread

    for_each_plugin(
        compose1(
            bind2nd(mem_fun(&DebuggerPlugin::on_attach), &thread),
            mem_fun_ref(&ImportPtr<DebuggerPlugin>::get)));

    if (firstThread || thread.is_execed())
    {
        // restore breakpoints and other settings if necessary
        // NOTE: this needs to happen AFTER all plug-ins have
        // been notified, so that debug symbol readers are properly
        // initialized; restoring breakpoints may trigger
        // address-line lookups

        read_process_history(*CHKPTR(thread.process()));

        if (thread.is_live())
        {
            // helpful when stepping over functions that throw
            set_breakpoint_at_catch(&thread);

            if ((options() & OPT_BREAK_ON_THROW))
            {
                set_breakpoint_at_throw(&thread);
            }
        }
    }

    if (thread.is_execed())
    {
        set_have_new_program(true);
    }

    // give the plugins a chance to update their states
    message("attached", MSG_UPDATE, &thread, false);
}


typedef ExprObserver::Events Events;

/**
 * Notify interested parties of events that occur while
 * an expression evaluation is pending
 */
static bool notify_expr_events(
    Thread& thread,
    addr_t addr,
    const RefPtr<ExprObserver>& obs)
{
    bool handled = false;

    if (obs)
    {
        Events events = obs->events();

        if (!thread.is_stopped_by_debugger())
        {
            Events::const_iterator i = events.begin();
            for (; i != events.end(); ++i)
            {
                if (RefPtr<ExprEvents> e = i->ref_ptr())
                {
                    handled |= e->on_event(&thread, addr);
                }
            }
        }
    }
    return handled;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::cleanup(Thread& thread)
{
    DebuggerBase::cleanup(thread);

    dbgout(0) << __func__ << ": sending on_detach to plugins" << endl;

    for_each_plugin(
        compose1(
            bind2nd(mem_fun(&DebuggerPlugin::on_detach), &thread),
            mem_fun_ref(&ImportPtr<DebuggerPlugin>::get)));

    if (breakPointMgr_)
    {
        breakPointMgr_->on_thread_unmanage(thread);
    }

    if (TargetManager::empty())
    {
        dbgout(0) << __func__ << ": detaching" << endl;
        detach();
    #if 0
        if (thread.is_forked())
        {
            quit();
        }
    #endif
    }
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::start_at_main(Thread& thread)
{
    bool result = false;

    if (Runnable* runnable = interface_cast<Runnable*>(&thread))
    {
        if ((options() & OPT_START_AT_MAIN))
        {
            RefPtr<SymbolMap> symbols = thread.symbols();
            if (!symbols)
            {
                return false;
            }
            FunctionEnum syms;

            if (support_d_language(*this, thread))
            {
                symbols->enum_symbols("_Dmain", &syms);
            }
            if (syms.empty())
            {
                symbols->enum_symbols("main", &syms);
            }
            SymbolEnum::const_iterator i = syms.begin();
            for (; i != syms.end(); ++i)
            {
                set_temp_breakpoint(runnable, (*i)->addr());
                result = true;
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::on_event(Thread& thread)
{
    if (have_new_program())
    {
        set_have_new_program(false);

        if (start_at_main(thread))
        {
            resume();
        }
        else
        {
            schedule_interactive_mode(&thread, E_DEFAULT);
        }
    }
    else
    {
        EventSchedule __scope__(sched_);

        const int signum = thread.signal();
        const addr_t addr = thread.program_count();
        string eventDesc;

        if (thread.is_exiting())
        {
            eventDesc = "Exiting";
        }
        else if (signum)
        {
            eventDesc = sig_name(signum);
        }
        if (!eventDesc.empty())
        {
            set_event_description(thread, addr, eventDesc.c_str());
        }
        if ((signum != 0) && (signum != SIGTRAP))
        {
            //
            // have a user-defined action handler? Note that SIGTRAP
            // handlers are always ignored (for speed, and to reduce
            // possible interference with the inner working of the engine).
            //
            if (RefPtr<BreakPointAction> act = get_sig_action(signum))
            {
                if (!act->execute(&thread, NULL))
                {
                    // remove the action unless it was changed
                    // from within the execute() method

                    if (get_sig_action(signum) == act.get())
                    {
                        set_sig_action(signum, NULL);
                    }
                }
            }
        }
        RunnableImpl* runnable = interface_cast<RunnableImpl*>(&thread);

        bool breakPointHandled = false;

        if (runnable && (signum == SIGTRAP))
        {
            breakPointHandled = try_breakpoints(*runnable, addr);
        }

        if (!breakPointHandled)
        {
            if (thread.is_exiting())
            {
                schedule_interactive_mode(&thread, E_THREAD_EXITING);
            }
            else if (signum == SIGTRAP)
            {
                if (runnable)
                {
                    // If we got a SIGTRAP and no breakpoint found
                    // at the program counter, it may be either because
                    // of single-stepping thru a thread or a system call.

                    on_single_step_or_syscall(*runnable, thread);
                }
                else
                {
                    // thread died because of an unhandled SIGTRAP?
                    set_event_description(thread, addr, "Thread received SIGTRAP");
                    schedule_interactive_mode(&thread, E_THREAD_STOPPED);
                }
            }
        }
        // if the signal occurred during an expression evaluation,
        // give the chance to handle the event to the observers registered
        // with the interpreter (the observers implement the ExprEvents
        // interface, see zdk/expr.h)

        if (notify_expr_events(thread, addr, exprObserver_))
        {
            if (signum == SIGTRAP)
            {
                sched_->exprEventsHandled_ = true;
            }
        }
        else if (signum != SIGTRAP)
        {
            if (thread.is_traceable())
            {
                schedule_interactive_mode(&thread, E_DEFAULT);
            }
        }

        exec_pending_actions(runnable, thread);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::begin_interactive_mode (
    Thread*     thread,
    EventType   event,
    Symbol*     symbol)
{
    if (thread && !is_silent())
    {
        addr_t programCounter = 0;

        // get the function at the current stack frame
        if (Frame* frame = thread_current_frame(thread))
        {
            if (!symbol)
            {
                symbol = frame->function();
            }
            programCounter = frame->program_count();
        }

        print_event_info(cout, *thread);
        if (symbol)
        {
            print_symbol(cout, programCounter, symbol);
            cout << endl;
        }
    }
}



////////////////////////////////////////////////////////////////
void DebuggerEngine::on_watchpoint(
    Runnable&   runnable,
    Thread&     thread,
    addr_t      addr,
    reg_t       condition)
{
#if !defined(__i386__) && !defined(__x86_64__)
// todo: define and use HAVE_WATCHPOINT_SUPPORT macro
 #warning watchpoints unsupported on this platform
#endif
    assert(condition == DebugRegs::BREAK_ON_DATA_WRITE
        || condition == DebugRegs::BREAK_ON_DATA_READ_WRITE);

    const char* detail =
        (condition == DebugRegs::BREAK_ON_DATA_WRITE)
        ? "memory write" : "memory access";

    set_event_description(thread, addr, "Watchpoint", detail);

    // the WatchType enum definitions matches DebugRegs::Condition
    WatchType type = static_cast<WatchType>(condition);

    assert(breakPointMgr_.get());
    RefPtr<WatchPoint> wp =
        breakPointMgr_->get_watchpoint(&thread, addr, type);

    if (wp.get() && wp->action_count())
    {
        schedule_actions(runnable, thread, *wp);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::on_single_step_or_syscall(

    Runnable&   runnable,
    Thread&     thread)
{
    const addr_t pc = thread.program_count();

    if (thread.single_step_mode())
    {
        // stepping over a source code line which
        // encompasses a range of addresses?

        if (thread.is_line_stepping())
        {
            runnable.set_single_step_mode(false);
        }
        else
        {
            dbgout(0) << "--- single-step ---" << endl;

            set_event_description(thread, pc, "Single stepping");
            schedule_interactive_mode(&thread, E_SINGLE_STEP);

            if (sched_)
            {
                // set flag to indicate we're single-stepping;
                // step mode will be turned off after
                // the pending breakpoint actions are executed

                sched_->step_ = true;
            }
            else
            {
                runnable.set_single_step_mode(false);
            }
        }
    }
    else
    {
        on_syscall(runnable, thread, pc);
    }
}

/**
 * Is this a valid syscall trap?
 * Determine if SIGTRAP was sent by a syscall -- should be
 * factored under Target when we really care about other OS-es.
 */
static bool valid_syscall_trap( const Thread& thread )
{
    const int status = thread.status();

    if ( WSTOPSIG(status) == (SIGTRAP | 0x80) )
    {
        return true;
    }

    // Heuristic for older Linux kernels that 
    // do not support PTRACE_O_TRACESYSGOOD.

    const pid_t sender = thread.get_signal_sender();
    const long ncall = thread.syscall_num();
   
    // sender <= 0: trap originated in kernel?
    return (ncall >= 0) && (sender <= 0 || sender == thread.lwpid());
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::on_syscall(

    Runnable&   /* runnable */,
    Thread&     thread,
    addr_t      programCount)
{
    static bool ignoreUnknownTrap =
        env::get_bool("ZERO_IGNORE_UNKNOWN_TRAP", true);

    // Threads are being resumed with PTRACE_SYSCALL until the
    // linker events breakpoint is set. PTRACE_CONT is used
    // afterwards.
    bool usingPtraceCont = false;

    if (RefPtr<Target> target = thread.target())
    {
        usingPtraceCont = target->has_linker_events();
    }

    // ignore traps caused by ptrace(PTRACE_SYSCALL)
    if ( !usingPtraceCont || !valid_syscall_trap(thread) )
    {
        // does not look like a legit syscall trap, break in 
        // the debugger only if the ZERO_IGNORE_UNKNOWN_TRAP
        // environment variable is set to "false"

        if ( !ignoreUnknownTrap && usingPtraceCont )
        {
            schedule_interactive_mode(&thread, E_THREAD_STOPPED);
        }
    }
    else
    {
        const long ncall = thread.syscall_num();

        ostringstream buf;

        buf << "System call #" << ncall;

        set_event_description(thread, programCount, buf.str());

        if (this->options() & (OPT_TRACE_SYSCALLS | OPT_BREAK_ON_SYSCALLS))
        {
            PluginList plugins = plugins_;
            for (PluginList::iterator i = plugins.begin(), end = plugins.end();
                i != end;
                ++i)
            {
                // notify plugins about the syscall trap event
                ActionScope scope(actionContextStack_, *i);
                (*i)->on_syscall(&thread, ncall);
            }
        }

        if (this->options() & OPT_BREAK_ON_SYSCALLS)
        {
            schedule_interactive_mode(&thread, E_SYSCALL);
        }
    }
}


/**
 * Schedule software breakpoint
 */
bool DebuggerEngine::schedule_soft(
    Runnable&   runnable,
    Thread&     thread,
    BreakPoint* bptr,
    addr_t      addr)
{
    if (bptr && (bptr->type() != BreakPoint::HARDWARE))
    {
        if (bptr->is_enabled())
        {
            schedule_actions(runnable, thread, *bptr, addr);
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::try_breakpoints(Runnable& runnable, addr_t pc)
{
    if (!pc)
    {
        return false;
    }
    bool result = false;
    Thread& thread = *CHKPTR(runnable.thread());

    // Test for hardware breakpoints
    addr_t hardAddr = 0;
    reg_t cond = 0;

    if (DebugRegs* dbgregs = thread.debug_regs())
    {
        hardAddr = dbgregs->hit(&cond);
    }
    if (hardAddr)
    {
        if (thread.is_traceable())
        {
            on_hardware_break(runnable, thread, hardAddr, cond);
        }
        result = true;
    }
    else
    {
        // On the x86 software breakpoints are "exceptions",
        // i.e.  a signal is raised AFTER the instruction
        // is executed (whereas hardware breakpoints are
        // "faults", i.e. the signal is raised BEFORE the
        // instruction is executed).
        PROGRAM_COUNT_BACKOUT(pc);
    }
    RefPtr<BreakPoint> bpnt;
    if (RefPtr<Process> process = runnable.process())
    {
        // search for global breakpoints first
        bpnt = get_breakpoint(*process, pc, &thread);
    }
    result |= schedule_soft(runnable, thread, bpnt.get(), pc);
    return result;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::on_hardware_break(
    Runnable&   runnable,
    Thread&     thread,
    addr_t      addr,
    reg_t       condition)
{
    assert(thread.is_traceable());
    const addr_t pc = thread.program_count();

    // Hardware breakpoints are faults, i.e. we get a
    // SIGTRAP before the instruction is executed, which
    // means that for a code break point, the address
    // must equal the program counter.

    if (addr != pc)
    {
        assert(condition != DebugRegs::BREAK_ON_INSTRUCTION);
        // handle memory breakpoint:
        on_watchpoint(runnable, thread, addr, condition);
    }
    else
    {
        RefPtr<BreakPoint> bptr = get_breakpoint(thread, addr);
        if (!bptr)
        {
            // this is not expected to happen -- if it does,
            // then there is a programmer mistake somewhere!

            ostringstream err;
            err << __func__ << ": get_breakpoint(addr=" << hex << addr;
            err << ", thread=" << dec << thread.lwpid() << ") failed!";
            throw logic_error(err.str());
        }
        assert(bptr->type() == BreakPoint::HARDWARE);
        if (verbose())
        {
            // print breakpoint
            interface_cast<BreakPointBase&>(*bptr).print(clog);
        }
        set_event_description(thread, pc, "Hardware Breakpoint");
        schedule_actions(runnable, thread, *bptr);
    }
}


////////////////////////////////////////////////////////////////
static void reenable_breakpoint(
    Runnable& runnable,
    Thread& thread,
    BreakPoint& bpnt)
{
    if (bpnt.is_enabled())
    {
        return;
    }
    const BreakPoint::Type type = bpnt.type();

    if (type != BreakPoint::HARDWARE
        && thread_is_attached(thread)
        && thread.program_count() == bpnt.addr())
    {
        // force debugged thread out of the breakpoint
        runnable.step_instruction();
    }

    // for global breakpoints we have to try to re-enable
    // even if the thread is gone
    if (thread_is_attached(thread) || (type == BreakPoint::GLOBAL))
    {
        bpnt.enable();
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::exec_pending_actions(
    RunnableImpl*   runnable,
    Thread&         thread)
{
    if (!sched_)
    {
        return; // nothing pending
    }
#if !NDEBUG
    addr_t addr = 0;
#endif
    // execute pending breakpoint actions
    BreakPointList pending = CHKPTR(sched_)->pending_;

    while (!pending.empty())
    {
        const RefPtr<BreakPoint>& bpnt = pending.front();
        assert(bpnt);

        // pending breakpoints should shared the same address
        assert(addr == 0 || bpnt->addr() == addr);
#if !NDEBUG
        addr = bpnt->addr();
#endif
        // are we still attached to this thread?
        if (thread_is_attached(thread))
        {
            execute_actions(*CHKPTR(runnable), thread, *bpnt);
        }
        pending.pop_front();
    }

    if (runnable)
    {
        // turn flag off so that we can compare after i
        // interactive mode completes
        runnable->set_single_step(false);
    }

    if (sched_->interactive_        &&
        !sched_->exprEventsHandled_ &&
        thread_is_attached(thread))
    {
        begin_interactive_mode(&thread, sched_->eventType_);
    }
    if (runnable && CHKPTR(sched_)->step_ && !runnable->single_step())
    {
        // if the flag inside RunnableImpl is set, it means that
        // set_single_step_mode(true) has been called from within
        // interactive mode; in that case we do not reset the
        // single stepping, since it seems that the some other
        // code specifically wants the thread to continue in single
        // step

        runnable->set_single_step_mode(false);
    }

    // enable pending breakpoints
    pending = CHKPTR(sched_)->pendingEnable_;
    while (!pending.empty())
    {
        RefPtr<BreakPoint> bptr = pending.front();

        // pending breakpoints should shared the same address
        assert(bptr->addr() == addr);
        reenable_breakpoint(*CHKPTR(runnable), thread, *bptr);
        pending.pop_front();
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::schedule_actions(
    Runnable&   runnable,
    Thread&     thread,
    BreakPoint& bpnt)
{
    // turn breakpoint off temporarily; will re-enable after
    // the actions associated with the breakpoint are executed
    // see execute_actions()
    bpnt.disable();

    if (!thread.is_traceable())
    {
        dbgout(0) << __func__ << ": " << thread.lwpid()
                  << " has debug events disabled." << endl;
    }
    else if (sched_)
    {
        dbgout(0) << __func__ << ": breakpoint pending" << endl;
        sched_->pending_.push_back(&bpnt);
    }
    else
    {
        execute_actions(runnable, thread, bpnt);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::schedule_actions(
    Runnable&   runnable,
    Thread&     thread,
    BreakPoint& bpnt,
    addr_t      pc      // program counter
    )
{
    assert(pc == bpnt.addr());

    // if we get here, we must have tested for hardware
    // breakpoints already
    assert(bpnt.type() != BreakPoint::HARDWARE);
    assert(pc == thread.program_count()
        || pc == thread.program_count() - 1);

    dbgout(1) << runnable.pid() << ": set_program_count("
              << hex << pc << dec << ")" << endl;

    runnable.set_program_count(pc);

    if (bpnt.enum_actions("__throw") || bpnt.enum_actions("__throw_once"))
    {
        set_event_description(thread, pc, "Exception");
    }
    else
    {
        set_event_description(thread, pc, "Breakpoint");
    }
    schedule_actions(runnable, thread, bpnt);
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::execute_actions(
     Runnable&      runnable,
     Thread&        thread,
     BreakPoint&    bpnt)
{
    dbgout(0) << __func__ << ": " << (void*)bpnt.addr() << endl;

    // just in case we hit a breakpoint while single-stepping:
    if (thread.single_step_mode())
    {
        dbgout(0) << "single_step_mode is on." << endl;
        on_single_step_or_syscall(runnable, thread);
    }

#ifdef DEBUG
    const BreakPoint::Type type = bpnt.type();
    if (verbose() > 2)
    {
        if (type == BreakPoint::SOFTWARE)
        {
            DebuggerBase::on_event(thread);
        }
        interface_cast<BreakPointBase&>(bpnt).print(clog);
    }
#endif
    bpnt.execute_actions(&thread);

    if (bpnt.action_count() == 0)
    {
        dbgout(0) << "removing empty breakpoint: ";
        dbgout(0) << (void*)bpnt.addr() << endl;

        if (breakPointMgr_.get())
        {
            breakPointMgr_->erase(&bpnt);
        }
        else
        {
            assert(!is_attached());
        }
    }
    else
    {
        assert(sched_);
        if (sched_)
        {
            sched_->pendingEnable_.push_back(&bpnt);
        }
        else
        {
            reenable_breakpoint(runnable, thread, bpnt);
        }
    }
}


////////////////////////////////////////////////////////////////
SymbolTableEvents* DebuggerEngine::symbol_table_events()
{
    if (!symbolEvents_.get())
    {
        symbolEvents_.reset(new SymbolEvents(*this));
    }
    return symbolEvents_.get();
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::set_user_breakpoint(
    Runnable*   runnable,
    addr_t      addr,
    bool        enable,
    bool        perThread)
{
    Thread* thread = runnable ? runnable->thread() : NULL;

    if (!thread)
    {
        thread = get_thread(DEFAULT_THREAD);
    }
    if (!thread)
    {
        throw runtime_error("no thread");
    }
    runnable = get_runnable(thread);

    RefPtr<BreakPoint> bpnt;
    if (perThread)
    {
        bpnt = get_breakpoint(*thread, addr);
    }
    else if (RefPtr<Process> process = thread->process())
    {
        bpnt = get_breakpoint(*process, addr);
    }
    //
    // breakpoint already exists, and has associated user actions?
    //
    if (bpnt && bpnt->enum_actions("USER") != 0)
    {
        // The breakpoint might be currently disabled;
        // if we're allowed to enable breakpoints...
        if (enable)
        {
            // ... try to enable -- enable_actions returns false
            // if all actions are enabled already
            if (enable_user_actions(*bpnt))
            {
                save_properties();
                on_insert(bpnt.get()); // notify observers
                return true;
            }
        }
        return false;
    }
    // the breakpoint does not exist, create new user action
    RefPtr<BreakPoint::Action> act = interactive_action("USER");
    const BreakPoint::Type type =
        perThread ? BreakPoint::PER_THREAD : BreakPoint::GLOBAL;

    BreakPointManager* mgr = breakpoint_manager();
    if (CHKPTR(mgr)->set_breakpoint(runnable, type, addr, act.get()))
    {
        take_snapshot();
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////////
BreakPoint* DebuggerEngine::set_temp_breakpoint (
    Runnable*   runnable,
    addr_t      addr)
{
    Thread* thread = CHKPTR(runnable)->thread();
    CHKPTR(thread);

    RefPtr<BreakPointAction> action = interactive_action("TEMP", false);
    RefPtr<BreakPoint> bpnt = get_breakpoint(*thread, addr);

    dbgout(0) << __func__ << ": " << hex << addr << dec << endl;

    if (bpnt)
    {
        dbgout(0) << __func__ << ": reusing " << bpnt->type() << endl;
        bpnt->add_action(action.get());
    }
    else
    {
        BreakPoint::Type type = BreakPoint::HARDWARE;
        if ((options() & OPT_HARDWARE_BREAKPOINTS) == 0
            || program_count(*thread) == addr  // hardware bkp at current PC may not be honored
         ) 
        {
            type = BreakPoint::EMULATED;
        }
        BreakPointManager* mgr = breakpoint_manager();
        if (mgr == NULL)
        {
            throw logic_error(__func__ + string(": no breakpoint manager"));
        }
        bpnt = mgr->set_breakpoint(runnable, type, addr, action.get());
    }
    assert(!bpnt || bpnt->ref_count() > 1);
    return bpnt.get();
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::remove_user_breakpoint (
    pid_t proc,     // process id
    pid_t thread,   // lwpid of thread
    addr_t addr)
{
    size_t removedCount =
        remove_breakpoint_action(proc, thread, addr, "USER");

    // it is essential to update the history here -- otherwise,
    // if the module gets reloaded (after a fork(), for e.q.) the
    // breakpoint will be automatically restored, which is not
    // the desired behavior
    take_snapshot();

    return removedCount;
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::remove_breakpoint_action(
    pid_t       procID,
    pid_t       threadID,
    addr_t      addr,
    const char* actionName)
{
    size_t removedCount = 0;

    if (breakPointMgr_)
    {
        removedCount =
            breakPointMgr_->remove_breakpoint_actions(procID,
                                                      threadID,
                                                      addr,
                                                      actionName);
    }
    return removedCount;
}



namespace
{
    /**
     * Helper functor for DebuggerEngine::remove_breakpoint_action.
     */
    CLASS ActionRemover : public EnumCallback<volatile BreakPoint*>
    {
    public:
        ActionRemover
        (
            BreakPointManagerGroup& mgr,
            BreakPoint::Action* action
        )
        : mgr_(mgr), action_(action)
        { }

        void notify(volatile BreakPoint* breakpoint)
        {
            assert(breakpoint);
            breakpoint->remove(action_);

            if (breakpoint->enum_actions() == 0)
            {
                mgr_.erase(const_cast<BreakPoint*>(breakpoint));
            }
        }

    private:
        BreakPointManagerGroup& mgr_;
        BreakPoint::Action* action_;
    };
}


////////////////////////////////////////////////////////////////
void
DebuggerEngine::remove_breakpoint_action(BreakPoint::Action* action)
{
    if (breakPointMgr_.get())
    {
        ActionRemover remover(*breakPointMgr_, action);

        breakPointMgr_->enum_breakpoints(&remover);
        breakPointMgr_->enum_watchpoints(&remover);
    }
}


/**
 * Debugged program will stop in debugger when the
 * specified memory address is accessed; the WatchType
 * parameter specifies the type of memory access.
 *
 * @return false if watchpoint could not be set (possibly
 * because all hardware debug registers are in use).
 */
bool DebuggerEngine::set_watchpoint(
    Runnable*   runnable,
    WatchType   type,
    bool        global,
    addr_t      addr)
{
    RefPtr<MemoryWatch> action(new MemoryWatch());

    BreakPoint* bpnt = breakpoint_manager()->set_watchpoint(
        runnable, type, global, addr, action.get());
    if (bpnt)
    {
        action->set_owner(interface_cast<WatchPoint&>(*bpnt));
        return true;
    }

    return false;
}


/**
 * Execution stops when a variable assumes a given value.
 * The value is specified as a string, and will be internally
 * converted to the appropriate datatype.
 * @note Currently, this operation works for fundamental types
 * only. A logic_error exception will be thrown for aggregate
 * types such as classes and arrays.
 *
 * @return false if watchpoint could not be set (possibly
 * because all hardware debug registers are in use).
 */
bool DebuggerEngine::break_on_condition (
    Runnable*       runnable,
    DebugSymbol*    dsym,
    RelType         relop,
    SharedString*   value,
    bool            global)
{
    assert(runnable);
    assert(dsym);
    assert(value);

    RefPtr<ValueWatch> action(new ValueWatch(dsym, relop, value));

    addr_t addr = dsym->addr();
    assert(dsym->type());

    // C/C++ require types to be at least one-byte long
    assert(dsym->type()->size());
    addr += (dsym->type()->size() - 1);

    BreakPoint* bpnt = breakpoint_manager()->set_watchpoint(
        runnable,
        WATCH_WRITE,
        global,
        addr,
        action.get());

    if (bpnt)
    {
        action->set_owner(interface_cast<WatchPoint&>(*bpnt));
        return true;
    }

    return false;
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::remove_watchpoint_action(BreakPoint::Action* action)
{
    //
    // todo
    //
    return false;
}


namespace
{
    /**
     * Arranges for the next line to be executed
     * upon hitting a break point.
     */
    class ZDK_LOCAL ActionNext : public ZObjectImpl<BreakPoint::Action>
    {
        typedef bool (DebuggerEngine::*Next)(Thread*, addr_t, addr_t);

        DebuggerEngine& engine_;
        Next next_;
        addr_t to_;

    public:
        ActionNext(DebuggerEngine& eng, Next next, addr_t to)
            : engine_(eng), next_(next), to_(to)
        { }

    private:
        const char* name() const
        {
            return "NEXT";
        }
        bool execute(Thread* thread, BreakPoint*)
        {
            (engine_.*next_)(thread, 0, to_);

            return false; // remove action after executing it
        }
        word_t cookie() const { return 0; }
    };

    /**
     * Arranges for the next machine-instruction to
     * be executed upon hitting a break point.
     */
    class ZDK_LOCAL ActionStep : public ZObjectImpl<BreakPoint::Action>
    {
        const char* name() const
        {
            return "STEP";
        }
        bool execute(Thread* thread, BreakPoint*)
        {
            get_runnable(thread)->set_single_step_mode(true);
            return false; // remove action after executing it
        }
        word_t cookie() const { return 0; }
    };
} // namespace


/**
 * Hack: if no info is present to tell what the next
 * instruction address is, peek at current the opcode
 * and then assume that the next address is the current
 * one plus the length of the current opcode.
 *
 * @note does not work with the objdump disassembler
 * plug-in, because it does not return the length of
 * disassembled sequence
 */
static addr_t next_addr(Disassembler* disasm, Thread* thread, addr_t current)
{
    addr_t result = 0;

#if defined(__i386__) || defined(__x86_64__)
    struct ZDK_LOCAL Helper : public Disassembler::SourceCallback
    {
        size_t len_;
        Helper() : len_(0) { }

        vector<string>* notify(addr_t addr, size_t len)
        {
            if (!len_)
            {
                len_ = len;
            }
            return NULL;
        }
    };

    if (disasm)
    {
        Helper helper;
        uint8_t buf[2 * sizeof (word_t)];
        size_t len = sizeof buf / sizeof (word_t);

        CHKPTR(thread)->read_code(current, (word_t*)buf, len, &len);

        if (len /* && (buf[0] == 0xe8 || buf[9] == 0x9a) */)
        {
            RefPtr<SharedString> fname;
            if (const char* name = thread->filename())
            {
                fname = shared_string(name);
            }

            len *= sizeof (word_t);
            len = disasm->disassemble(current,
                                      0,
                                      buf,
                                      len,
                                      fname.get(),
                                      NULL,
                                      NULL,
                                      &helper);
            result = current + (helper.len_ ? helper.len_ : len);
        }
    }
#endif // __i386__ || __x86_64__
    return result;
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::next(Thread* thread, addr_t from, addr_t to)
{
    assert(thread);

    Runnable* runnable = get_runnable(thread);
    assert(runnable); // by get_runnable's contract

    runnable->set_stepping(false);
    bool result = false; // true means: resume debugged program

    const addr_t pc = program_count(*thread);
    if (from == 0)
    {
        from = pc;
    }

    // if FROM is not the current program-counter, execute
    // until the FROM address is hit, then call this function
    // again (via ActionNext)

    if (from != pc)
    {
        RefPtr<BreakPoint::Action> action =
            new ActionNext(*this, &DebuggerEngine::next, to);

        // We might be in a different stack frame:
        // just schedule to execute this command
        // as soon as the 'pc' address is hit.

        breakpoint_manager()->set_breakpoint(runnable,
                                             BreakPoint::HARDWARE,
                                             from,
                                             action.get());
        result = true;
    }
    else
    {
        RefPtr<SymbolMap> symbols = thread->symbols();
        if (!symbols)
        {
            return false;
        }
        if (RefPtr<Symbol> sym = symbols->lookup_symbol(from))
        {
            if (to == 0)
            {
                to = symbol_table_events()->next_line(sym);
            }
            if (to == 0)
            {
                // attempt to determine next addr from disasm
                to = next_addr(disasm_.get(), thread, from);
            }
            if (to == 0)
            {
                // run until current function returns
                to = thread->ret_addr();
            }
            else
            {
                // memorize the site where we start;
                // if we land on the same file and line,
                // call this function again --
                // see break_into_interactive_mode and next_line_hack
                runnable->set_next(sym.get());
            }
            if (to && from != to)
            {
                ThreadImpl& impl = interface_cast<ThreadImpl&>(*thread);
                impl.step_thru(from, to, current_action_context());

                dbgout(0) <<(void*)from << " step to " <<(void*)to << endl;
                result = true;
            }
            else
            {
                if (!to)
                {
                    cerr << "Cannot determine next line's address\n";
                }
                runnable->set_single_step_mode(true, current_action_context());
                result = true;
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::step(
    Thread* thread,
    addr_t  from,
    bool    machCode // step one machine instruction, if true;
                     // otherwise, step one source line
    )
{
    if (!machCode && next(thread, from, 0))
    {
        ThreadImpl& impl = interface_cast<ThreadImpl&>(*thread);
        impl.set_step_into(true);
        impl.set_stepping(true);
        return;
    }
    const addr_t pc = program_count(*thread);
    if (from == 0)
    {
        from = pc;
    }

    Runnable* runnable = get_runnable(thread);
    if (from != pc)
    {
        RefPtr<BreakPoint::Action> action = new ActionStep;
        breakpoint_manager()->set_breakpoint(
            runnable,
            BreakPoint::HARDWARE,
            from,
            action.get());
    }
    else
    {
        runnable->set_single_step_mode(true, current_action_context());
        if (sched_)
        {
            // prevent turning off single stepping when thread resumes
            sched_->step_ = false;
        }
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::step(
    Thread*     thread,
    StepMode    mode,
    addr_t      from,
    addr_t      to)
{
    switch (mode)
    {
    case STEP_INSTRUCTION:
        dbgout(0) << "STEP_INSTRUCTION" << endl;
        step(thread, from, true);
        break;

    case STEP_SOURCE_LINE:
        dbgout(0) << "STEP_SOURCE_LINE" << endl;
        step(thread, from, false);
        break;

    case STEP_OVER_SOURCE_LINE:
        dbgout(0) << "STEP_OVER_SOURCE_LINE" << endl;
        next(thread, from, 0);
        break;

    case STEP_OVER_INSTRUCTION:
        dbgout(0) << "STEP_OVER_INSTRUCTION" << endl;
        next(thread, from, to);
        break;

    case STEP_RETURN:
        get_runnable(thread)->step_until_current_func_returns();
        break;

    case STEP_NONE:
        assert(false);
        break;
    }
}


namespace
{
    /**
     * Implementation of the EnumCallback<BreakPoint> interface,
     * for printing break point information to an output stream.
     */
    class ZDK_LOCAL BreakPointPrinter : public EnumCallback<volatile BreakPoint*>
    {
    public:
        explicit BreakPointPrinter(ostream& outs)
            : outs_(outs), pid_(0)
        { }

    protected:
        void notify(volatile BreakPoint* bp)
        {
            assert(bp); // pre-condition

            if (Thread* thread = bp->thread())
            {
                if (Process* proc = thread->process())
                {
                    if (proc->pid() != pid_)
                    {
                        pid_ = proc->pid();
                        outs_ << "--- " << pid_ << ": ";
                        outs_ << proc->name() << " ---\n";
                    }
                }
            }
            BreakPointBase* base =
              interface_cast<BreakPointBase*>(const_cast<BreakPoint*>(bp));

            if (base)
            {
                base->print(outs_);
            }
        }

    private:
        ostream& outs_;
        pid_t pid_;
    };
} // namespace


////////////////////////////////////////////////////////////////
void DebuggerEngine::print_breakpoints(ostream& outs, pid_t pid) const
{
    if (breakPointMgr_)
    {
        BreakPointPrinter callback(outs << endl);
        if (pid)
        {
            BreakPointManager* mgr = breakPointMgr_->get_manager(pid);
            if (mgr)
            {
                mgr->enum_breakpoints(&callback);
            }
        }
        else
        {
            // call the printer for all breakpoints:
            breakPointMgr_->enum_breakpoints(&callback);
        }
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::schedule_interactive_mode(
    Thread*     thread,
    EventType   event)
{
    if (event == E_EVAL_COMPLETE)
    {
        const addr_t pc = CHKPTR(thread)->program_count();

        RefPtr<BreakPoint> bpnt;

        // lookup global breakpoints first

        if (RefPtr<Process> process = thread->process())
        {
            bpnt = get_breakpoint(*process, pc);
        }
        if (!bpnt)
        {
            // lookup per-thread breakpoints
            bpnt = get_breakpoint(*thread, pc);
        }
        if (bpnt && bpnt->is_enabled() && has_enabled_actions(*bpnt))
        {
            // at this point we have a breakpoint,
            // which should only be true for Runnable threads
            // (and NEVER for corefiles)
            schedule_actions(*get_runnable(thread), *thread, *bpnt);
        }
    }
    if (thread && !thread->is_traceable())
    {
        dbgout(0) << __func__ << ": " << thread->lwpid()
                  << " has debug events disabled." << endl;
    }
    // event pending? schedule to break in debugger
    // as soon as pending events are processed
    else if (sched_)
    {
        sched_->interactive_ = true;
        sched_->eventType_ = event;
    }
    else
    {
        begin_interactive_mode(thread, event);
    }
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::next_line_hack(
    ThreadImpl& thread,
    const RefPtr<BreakPoint>& bptr,
    EventType eventType)
{
    RefPtr<Symbol> sym;

    if ((eventType == E_DEFAULT) && bptr->symbol())
    {
        sym = thread.get_next();
    }

    if (sym)
    {
        // Make sure we haven't landed on the same
        // source line where we started; it can happen
        // for complex source lines that generate several
        // machine instructions.
        assert(bptr->symbol());

        if (bptr->addr() != sym->addr()
            && bptr->symbol()->line()
            && bptr->symbol()->line() == sym->line()
            && bptr->symbol()->file()
            && bptr->symbol()->file()->is_equal2(sym->file())
          )
        {
            ActionScope sc(actionContextStack_, thread.action_context());

            const bool step = thread.is_stepping();

            next(&thread, bptr->addr(), 0);
            thread.set_next(bptr->symbol());

            dbgout(0) << __func__ << ": " << (void*)bptr->addr()
                      << " stepping from:"<< (void*)sym->addr()
                      << " line=" << sym->line() << endl;
            if (step)
            {
                thread.set_step_into(true);
            }
            return true;
        }
    }
    thread.set_next(NULL);
    return false;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::break_into_interactive_mode(
    RefPtr<Thread>      thread,
    RefPtr<BreakPoint>  bptr)
{
    // pre-conditions
    assert(thread);
    assert(bptr);

    dbgout(1) << __func__ << ": " << bptr->type() << " break" << endl;

    // Are there any user-defined actions associated with
    // this breakpoint? i.e. does this internal action overlap
    // with a user-inserted breakpoint?
    const EventType eventType =
        bptr->enum_actions("USER") ? E_THREAD_BREAK : E_DEFAULT;

    ThreadImpl& threadImpl = interface_cast<ThreadImpl&>(*thread);

    // hack: did we get here as a result of a 'next' command?
    // if yes, repeat the 'next' command
    if (next_line_hack(threadImpl, bptr, eventType))
    {
        return;
    }

    if (eventType != E_THREAD_BREAK)
    {
        threadImpl.check_return_value();
    }
    // prevent spurious stops when stepping over breakpoints
    if (!threadImpl.is_addr_in_step_range(bptr->addr())
       || bptr->enum_actions("STEP_OVER"))
    {
        schedule_interactive_mode(&threadImpl, eventType);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::break_at_catch(
    RefPtr<Thread>      thread,
    RefPtr<BreakPoint>  breakpoint
    )
{
    // pre-conditions
    assert(thread);
    assert(breakpoint);

    const addr_t addr = breakpoint->addr();
    set_event_description(*thread, addr, "Exception caught");

    if (thread->is_line_stepping())
    {
        if (RefPtr<StackTrace> trace = thread->stack_trace())
        {
            ThreadImpl& threadImpl = interface_cast<ThreadImpl&>(*thread);

            const size_t size = trace->size();
            for (size_t i = 0; i != size; ++i)
            {
                const addr_t addr = CHKPTR(trace->frame(i))->program_count();

                if (threadImpl.is_addr_in_step_range(addr, true))
                {
                    return;
                }
            }
            set_temp_breakpoint(&threadImpl.runnable(), thread->ret_addr());
        }
    }
    else if ((options() & OPT_BREAK_ON_THROW))
    {
        ThreadImpl& threadImpl = interface_cast<ThreadImpl&>(*thread);
        schedule_interactive_mode(&threadImpl, E_THREAD_BREAK);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::break_at_throw(
    RefPtr<Thread>      thread,
    RefPtr<BreakPoint>  breakpoint
    )
{
    addr_t addr = breakpoint->addr();
    set_event_description(*thread, addr, "Throwing exception");

    break_into_interactive_mode(thread, breakpoint);
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::publish_event(Thread* thread, EventType type)
{
    bool result = false;

    // Loop thru a copy of the actual list; this way, if plugins
    // get shut down from within the on_event call, memory does
    // not go away from underneath us.
    PluginList plugins(plugins_);

    DebuggerPlugin* actor = thread
        ? interface_cast<DebuggerPlugin*>(thread->action_context())
        : NULL;
    if (actor)
    {
        ActionScope scope(actionContextStack_, actor);
        result = actor->on_event(thread, type);
    }

    if (!result)
    {
        // Send the notification to all registered plug-ins; as soon
        // as one is interested in overriding the handling of the
        // breakpoint, stop any further processing. This behavior
        // enables a custom command-line interface or UI to take over.

        PluginList::const_iterator i = plugins.begin();
        for (; (i != plugins.end()) && !result; ++i)
        {
            if (i->get() != actor)
            {
                ActionScope scope(actionContextStack_, *i);
                result = (*i)->on_event(thread, type);
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::on_resumed()
{
    DebuggerBase::on_resumed();

    dbgout(2) << __func__ << ": notify plug-ins" << endl;

    for_each_plugin(
        compose1(
            mem_fun(&DebuggerPlugin::on_program_resumed),
            mem_fun_ref(&ImportPtr<DebuggerPlugin>::get)));
}


////////////////////////////////////////////////////////////////
BreakPointManager* DebuggerEngine::breakpoint_manager(pid_t pid)
{
    if (pid == 0)
    {
        return breakPointMgr_.get();
    }
    else if (!breakPointMgr_)
    {
        return NULL;
    }
    return breakPointMgr_->get_manager(pid);
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::on_insert(volatile BreakPoint* breakPoint)
{
    // Notify all plugins that a breakpoint was inserted
    for_each_plugin(
        compose1(
            bind2nd(
                mem_fun(&DebuggerPlugin::on_insert_breakpoint),
                breakPoint),
            mem_fun_ref(&ImportPtr<DebuggerPlugin>::get)));
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::on_remove(volatile BreakPoint* breakPoint)
{
    // Notify all plugins that a breakpoint is being removed
    for_each_plugin(
        compose1(
            bind2nd(
                mem_fun(&DebuggerPlugin::on_remove_breakpoint),
                breakPoint),
            mem_fun_ref(&ImportPtr<DebuggerPlugin>::get)));
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::enum_plugins(EnumCallback<DebuggerPlugin*>* events)
{
    if (events)
    {
        for_each_plugin(
            compose1(
                bind1st(mem_fun(&EnumCallback<DebuggerPlugin*>::notify),
                events),
            mem_fun_ref(&ImportPtr<DebuggerPlugin>::get)));
    }
    return plugins_.size();
}


namespace
{
    /**
     * Helper class used by DebuggerEngine::enum_globals
     */
    class ZDK_LOCAL GlobalsHelper : public EnumCallback<DebuggerPlugin*>
    {
    public:
        GlobalsHelper
        (
            Thread*             thread,
            const char*         name,
            Symbol*             func,
            DebugSymbolEvents*  events,
            LookupScope         scope,
            bool                enumFuncs
        )
          : thread_(thread)
          , name_(name)
          , func_(func)
          , events_(events)
          , count_(0)
          , scope_(scope)
          , enumFuncs_(enumFuncs)
          , typeSystem_(0)
        {
            if (thread_)
            {
                typeSystem_ = interface_cast<TypeSystem*>(thread_->process());
            }
        }

        size_t count() const { return count_; }

    private:
        void notify(DebuggerPlugin*);

        void try_fully_qualified_name(DebugInfoReader&);

    private:
        Thread* const       thread_;
        const char*         name_;
        Symbol* const       func_;
        DebugSymbolEvents*  events_;
        size_t              count_;
        const LookupScope   scope_;
        const bool          enumFuncs_;
        TypeSystem*         typeSystem_;
    };
} // namespace


////////////////////////////////////////////////////////////////
void GlobalsHelper::notify(DebuggerPlugin* plugin)
{
    DebugInfoReader* reader = interface_cast<DebugInfoReader*>(plugin);
    if (!reader)
    {
        return;
    }

    size_t count = 0;
    if (func_)
    {
        count = reader->enum_globals(   thread_,
                                        name_,
                                        func_,
                                        events_,
                                        scope_,
                                        enumFuncs_);
    }
    if (count)
    {
        count_ += count;
    }
    else if (name_)
    {
        try_fully_qualified_name(*reader);
    }
}


////////////////////////////////////////////////////////////////
void GlobalsHelper::try_fully_qualified_name(DebugInfoReader& reader)
{
    assert(thread_);

    const char* p = strrchr(name_, ':');

    if (!p || *--p != ':')
    {
        return;
    }
    assert(p >= name_);

    string temp(name_, p);

#ifdef DEBUG
    if (thread_->debugger()->verbose())
    {
        clog << __func__ << ": " << temp << endl;
    }
#endif
    const addr_t addr = func_ ? func_->addr() : 0;

    RefPtr<DataType> type =
        reader.lookup_type(thread_, temp.c_str(), addr, scope_);

    if (!type)
    {
        return;
    }

    ClassType* klass = interface_cast<ClassType*>(type.get());
    if (!klass)
    {
        return;
    }

    p += 2;
    const size_t n = klass->member_count();
    for (size_t i(0); i != n; ++i)
    {
        const Member* mem = klass->member(i);

        if (!CHKPTR(mem->name())->is_equal(p))
        {
            continue;
        }
        if (!mem->is_static())
        {
            throw runtime_error(string(name_) + " is non-static");
        }

        SymbolEnum s;

        RefPtr<SymbolMap> symbols = thread_->symbols();
        if (!symbols)
        {
            // symbols can be NULL only during the
            // attach phase, when the ThreadImpl object
            // is being initialized
            continue;
        }
        symbols->enum_symbols(name_, &s);
        if (s.empty())
        {
            continue;
        }
        if (s.size() > 1)
        {
            clog << "*** Warning: " << s.size();
            clog << " matches found for: " << name_ << endl;
        }
        const addr_t addr = s.front()->addr();

        assert(mem->type());

        if (typeSystem_)
        {
            RefPtr<DebugSymbol> dbgSym = typeSystem_->create_debug_symbol(
                    &reader,
                    thread_,
                    mem->type(),
                    mem->name(),
                    addr);
            dbgSym->read(events_);
            if (events_)
            {
                events_->notify(dbgSym.get());
            }
            ++count_;
        }
        break;
    }
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::enum_globals (
    Thread*             thread,
    const char*         name,
    Symbol*             func,
    DebugSymbolEvents*  events,
    LookupScope         scope,
    bool                enumFuncs)
{
    assert(name);
    assert(thread);

    if (scope == LOOKUP_PARAM)
    {
        throw invalid_argument(__func__ + string(": LOOKUP_PARAM"));
    }
    if (func == 0)
    {
        if (Frame* f = thread_current_frame(thread))
        {
            func = f->function();
        }
    }
    // dbgout(0) << "scope=" << scope << endl;

    GlobalsHelper h(thread, name, func, events, scope, enumFuncs);
    enum_plugins(&h);

    size_t numMatches = h.count();

    // if no debug symbol found, and functions are acceptable
    // in the result set, then fallback to the symbol table
    if (!numMatches && events && enumFuncs)
    {
        numMatches = enum_funcs(*thread, name, *events);
    }
    // dbgout(1) << __func__ << ": " << numMatches << " matching" << endl;

    return numMatches;
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::enum_funcs (
    Thread& thread,
    const char* name,
    DebugSymbolEvents& events)
{
    SymbolEnum syms;

    SymbolMap* symbols = CHKPTR(thread.symbols());

    symbols->enum_symbols(name, &syms);
    if (syms.empty())
    {
        symbols->enum_symbols(name, &syms, SymbolTable::LKUP_DYNAMIC);
    }

    if (!syms.empty())
    {
        TypeSystem& typeSystem = interface_cast<TypeSystem&>(thread);
        RefPtr<SharedString> fname = typeSystem.get_string(name);

        DataType* intType = GET_INT_TYPE(typeSystem, int);

        // if a match is found, assume it's and old-style C function,
        // which takes a variable number of arguments and returns int
        RefPtr<FunType> funType =
            typeSystem.get_fun_type(intType, 0, 0, true, false);

        assert(funType->has_variable_args());
        assert(!funType->is_return_type_strict());
        SymbolEnum::const_iterator i = syms.begin();
        for (; i != syms.end(); ++i)
        {
        #if DEBUG
            ZObjectScope scope;
            if (SymbolTable* table = (*i)->table(&scope))
            {
                dbgout(0) << (*i)->name() << " in " << table->filename() << endl;
            }
        #endif
            const addr_t addr = (*i)->addr();

            RefPtr<DebugSymbol> dbgSym = typeSystem.create_debug_symbol(
                                        NULL,   // synthesised, no reader
                                        &thread,
                                        funType.get(),
                                        fname.get(),
                                        addr,
                                        (*i)->file(),
                                        (*i)->line(),
                                        false); // not a return value

            events.notify(dbgSym.get());
        }
    }
    return syms.size();
}


namespace
{
/**
 * Helper callback sink, for DebuggerEngine::enum_locals.
 */
class ZDK_LOCAL Enumerator : public EnumCallback<DebuggerPlugin*>
{
    typedef DebugSymbolEvents Events;

    RefPtr<Thread>  thread_;
    Frame*          frame_;
    RefPtr<Symbol>  sym_;   // function in scope
    Events*         events_;
    size_t          count_; // variables enumerated
    string          name_;  // filter by name if not empty
    bool            paramOnly_;

    Enumerator(const Enumerator&);
    Enumerator& operator=(const Enumerator&);

    void notify(DebuggerPlugin* p)
    {
        assert(thread_.get());
        assert(sym_.get());

        if (count_)
        {
            return;
        }
        assert(p);

        if (DebugInfoReader* r = interface_cast<DebugInfoReader*>(p))
        {
            count_ = r->enum_locals(thread_.get(),
                                    name_.c_str(),
                                    frame_,
                                    sym_.get(),
                                    events_,
                                    paramOnly_);
        }
    }

public:
    virtual ~Enumerator() {}

    Enumerator
    (
        Thread* thread,
        Frame&  frame,
        Symbol* symbol,
        Events* events,
        const char* name,
        bool    paramOnly
    )
      : thread_(thread)
      , frame_(&frame)
      , sym_(symbol)
      , events_(events)
      , count_(0)
      , name_(name ? name : "")
      , paramOnly_(paramOnly)
    {
        assert(thread);
    }

    size_t count() const { return count_; }
};

} //namespace


static const string& errno_macro()
{
    static const string macro =
        env::get_string("ZERO_ERRNO", "*(__errno_location())");
    return macro;
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::enum_variables(
    Thread*             thread,
    const char*         name,
    Symbol*             func,
    DebugSymbolEvents*  events,
    LookupScope         scope,
    bool                enumFuncs)
{
    CHKPTR(thread);

    // get the current stack frame
    RefPtr<Frame> frame = thread_current_frame(thread);

    if (!func)
    {
        // if no function context given, assume the current scope
        if (frame)
        {
            func = frame->function();
        }
        if (!func)
        {
            return enumFuncs ? enum_funcs(*thread, name, *events) : 0;
        }
    }
    assert(func);

    size_t count = 0;

    if (name && (scope != LOOKUP_PARAM) && strcmp(name, "this") == 0)
    {
        scope = LOOKUP_LOCAL;
    }
    if ((scope != LOOKUP_LOCAL) && (scope != LOOKUP_PARAM))
    {
        count += enum_globals(thread, name, func, events, scope, enumFuncs);
    }
    if (func && frame)
    {
        const bool params = (scope == LOOKUP_PARAM);

        count += enum_locals(*thread, *frame, name, *func, events, params);

        if ((count == 0) && name && !params)
        {
            // not found, try member variables
            count += enum_members(*thread, *frame, name, *func, events);
        }
    }
    if (count == 0 && thread && events && name && strcmp(name, "errno") == 0)
    {
        emit_macro(*thread, *events, "errno", errno_macro());
        ++count;
    }
    return count;
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::enum_locals (
    Thread&             thread,
    Frame&              frame,
    const char*         name,
    Symbol&             func,
    DebugSymbolEvents*  events,
    bool                paramOnly // function parameters only
    )
{
    Enumerator cb(&thread, frame, &func, events, name, paramOnly);
    enum_plugins(&cb);

    size_t count = cb.count();

    if ((name == 0) || (*name == '\0'))
    {
        RefPtr<DebugSymbol> ret = thread.func_return_value();
        if (ret)
        {
            if (events)
            {
                ret->read(events);
                events->notify(ret.get());
                ++count;
            }
        }
    }
    dbgout(1) << __func__ << ": " << count << " matching" << endl;

    return count;
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::enum_members (
    Thread&             thread,
    Frame&              frame,
    const char*         name,
    Symbol&             func,
    DebugSymbolEvents*  events)
{
    DebugSymbolList list;
    DebugSymbolHelpers::Events tmp(list);

    // lookup "this" in the current scope
    if (enum_locals(thread, frame, "this", func, &tmp))
    {
        assert(!list.empty());

        RefPtr<DebugSymbol> thisp = list.front();
        CHKPTR(thisp)->read(NULL);

        if (thisp->enum_children(NULL) == 0)
        {
            return 0;
        }

        // lookup the instance referred by `this'
        if (RefPtr<DebugSymbol> child = lookup_child(*thisp, name))
        {
            RefPtr<DebugSymbol> child2 = child->clone();
            assert(child2->ref_count() == 1);

            events->notify(child2.get());

            return 1;
        }
    }
    return 0;
}


namespace
{
    class ErrnoWrapper;
    typedef ExprEventsWrapper<ErrnoWrapper> ExprEventsBase;

    /**
     * Force the name of the resulting symbol to be "errno" when
     * evaluating errno, even it is substituted for a different expression.
     *
     * @see DebuggerEngine::evaluate
     */
    class ErrnoWrapper : public ExprEventsWrapper<ErrnoWrapper>
    {
    public:
        explicit ErrnoWrapper(ExprEvents* events) : ExprEventsBase(events)
        { }
        ~ErrnoWrapper() throw()
        { }
        bool on_done(Variant* var, bool* interactive, DebugSymbolEvents* dse)
        {
            if (var)
            {
                DebugSymbol* sym = var->debug_symbol();
                if (DebugSymbolImpl* impl = interface_cast<DebugSymbolImpl*>(sym))
                {
                    impl->set_name(SharedStringImpl::create("errno"));
                }
            }
            return ExprEventsBase::on_done(var, interactive, dse);
        }
    };
}

////////////////////////////////////////////////////////////////
bool DebuggerEngine::evaluate (
    const char* expr,
    Thread*     thread,
    addr_t      addr, // not used
    ExprEvents* events,
    int         numericBase)
{
    assert(expr);
    assert(thread);

    string subst;
    if (strcmp(expr, "errno") == 0)
    {
        expr = errno_macro().c_str();
        events = new ErrnoWrapper(events);
    }

    if (events)
    {
        exprObserver_->add_events(*events);
    }

    // make an input stream for the expression
    // to be interpreted
    istringstream input(expr);

    RefPtr<Context> ctxt = ContextImpl::create(*thread, addr);
    RefPtr<Interp> interp = Interp::create(*ctxt, input);

    if (interp->run(events, numericBase) == Interp::EVAL_AGAIN)
    {
        return false;
    }
    return true;
}



////////////////////////////////////////////////////////////////
BreakPointAction* DebuggerEngine::get_sig_action(size_t signum) const
{
    if (signum < sigActions_.size())
    {
        return sigActions_[signum].get();
    }
    return 0;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::set_sig_action (
    size_t signum,
    BreakPointAction* action)
{
    // Thread-safety NOTE:
    // let the client code synchronize access -- consider
    // get_sig_action() above: even if we lock it, by the time
    // the pointer is returned, and _before_ the client code
    // wraps it into a RefPtr, a set_sig_action call from a
    // different thread may invalidate the pointer.

    if (sigActions_.size() <= signum)
    {
        throw_signal_out_of_range(__func__, signum);
    }
    sigActions_[signum] = action;
}


////////////////////////////////////////////////////////////////
namespace
{
    class ZDK_LOCAL AddrEventsAdapter : public EnumCallback<addr_t>
    {
        RefPtr<SymbolTable> table_;
        Debugger::AddrEvents* events_;

    public:
        AddrEventsAdapter(RefPtr<SymbolTable> table,
                           Debugger::AddrEvents* events)
            : table_(table)
            , events_(events)
        { }

        void notify(addr_t addr)
        {
            if (events_)
            {
                events_->notify(table_.get(), addr);
            }
        }
    };


    class ZDK_LOCAL LineToAddrHelper : public EnumCallback<SymbolTable*>
    {
    private:
        RefPtr<SharedString>        sourceFile_;
        size_t                      line_;
        vector<DebugInfoReader*>    debugInfo_;
        Debugger::AddrEvents*       events_;
        size_t                      result_;
        RefPtr<SharedString>        moduleName_;

    private:
        void notify(SymbolTable* table)
        {
            CHKPTR(table);
            SharedString* filename = CHKPTR(table)->filename();
            if (moduleName_ && !filename->is_equal2(moduleName_.get()))
            {
                return;
            }

            assert(table->addr() == 0);

            AddrEventsAdapter adapter(table, events_);

            vector<DebugInfoReader*>::const_iterator i = debugInfo_.begin(),
                                                   end = debugInfo_.end();
            for (ZObjectScope scope; i != end; ++i)
            {
                result_ += (*i)->line_to_addr(table->process(&scope),
                                              CHKPTR(filename),
                                              table->adjustment(),
                                              sourceFile_.get(),
                                              line_,
                                              &adapter);
            }
        }

    public:
        virtual ~LineToAddrHelper() throw() {}

        LineToAddrHelper
        (
            SharedString* sourceFile,
            size_t line,
            const vector<DebugInfoReader*>& debugInfo,
            Debugger::AddrEvents* events,
            const RefPtr<SharedString>& moduleName
        )
        : sourceFile_(sourceFile)
        , line_(line)
        , debugInfo_(debugInfo)
        , events_(events)
        , result_(0)
        , moduleName_(moduleName)
        { }

        size_t result() const { return result_; }
    };
}


/**
 * Attempt to figure out the build path from the current symbol.
 */
static void get_full_build_path(
    const Thread* thread,
    RefPtr<SharedString>& fullpath,
    SharedString*& sourceFile)
{
    if (Symbol* fun = thread_current_function(thread))
    {
        string path = CHKPTR(fun->file())->c_str();
        size_t delim = path.rfind('/');
        if (delim != string::npos)
        {
            path.erase(++delim);
            path += sourceFile->c_str();

            fullpath = shared_string(path);
            sourceFile = fullpath.get();
        }
    }
}



////////////////////////////////////////////////////////////////
size_t DebuggerEngine::line_to_addr(
    SharedString*   sourceFile,
    size_t          lineNumber,
    AddrEvents*     events,
    const Thread*   thread,
    bool*           cancelled)
{
    if (!thread)
    {
        thread = get_thread(DEFAULT_THREAD);
    }
    if (!thread)
    {
        assert(!is_attached());
        return 0;
    }
    assert(sourceFile);
    RefPtr<SharedString> fullpath;
    if (*sourceFile->c_str() != '/') // relative path?
    {
        get_full_build_path(thread, fullpath, sourceFile);
    }
    SymbolMap* symbols = CHKPTR(thread->symbols());
    if (!symbols)
    {
        assert(false); // should not happen!
        return 0;
    }
    size_t result = 0;
    vector<DebugInfoReader*> debugInfoReaders;

    RefPtr<SharedString> moduleName;
    RefPtr<Process> process = thread->process();
    if (RefPtr<TranslationUnit> unit =
        lookup_unit_by_name(process.get(), sourceFile->c_str()))
    {
        moduleName = unit->module_name();
    }
    else
    {
        dbgout(0) << __func__ << ": unit not found: "
                  << sourceFile->c_str() << endl;
    }
    //
    // first pass: iterate over all loaded modules
    //
    RefPtr<SymbolMap::LinkData> link = symbols->file_list();
    for (; link; link = link->next())
    {
        SharedString* filename = CHKPTR(link->filename());
        AddrEventsAdapter adapter(link->table(), events);

        // iterate over all debug info readers
        PluginList::iterator i = plugins_.begin(), end = plugins_.end();
        for (; i != end; ++i)
        {
            DebugInfoReader* debugInfo =
                interface_cast<DebugInfoReader*>((*i).get());
            if (!debugInfo)
            {
                continue;
            }
            dbgout(0) << __func__ << ": " << debugInfo->format() << endl;

            debugInfoReaders.push_back(debugInfo);
            if (moduleName && !filename->is_equal2(moduleName.get()))
            {
                continue;
            }
            assert(thread);
            result += debugInfo->line_to_addr(thread->process(),
                                              filename,
                                              link->adjustment(),
                                              sourceFile,
                                              lineNumber,
                                              &adapter);
        }
    }
    if (result == 0)
    {
        // second pass: look inside the dynamic libraries that are
        // needed, but not yet loaded
        LineToAddrHelper helperCB( sourceFile,
                                    lineNumber,
                                    debugInfoReaders,
                                    events,
                                    moduleName );

        symbols->enum_needed_tables(&helperCB);
        result = helperCB.result();
    }
    return result;
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::progress(const char* what, double percent, word_t cookie)
{
    PluginList::iterator i = plugins_.begin();
    for (; i != plugins_.end(); ++i)
    {
        ActionScope scope(actionContextStack_, *i);
        if (!(*i)->on_progress(what, percent, cookie))
        {
            return false;
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::critical_error(Thread* thread, const char* what)
{
    if (quitCommandPending_)
    {
        cerr << what << endl;
    }
    else
    {
        message(what, MSG_ERROR, thread);
    }
}



////////////////////////////////////////////////////////////////
bool DebuggerEngine::message (
    const char*     msg,
    MessageType     type,
    Thread*         thread,
    bool            async)
{
    bool result = false;

    PluginList tmp(plugins_);
    PluginList::iterator i = tmp.begin(), end = tmp.end();
    for (; i != end; ++i)
    {
        ActionScope scope(actionContextStack_, *i);
        result |= (*i)->on_message(msg, type, thread, async);
    }
    if (!result)
    {
        cerr << "***** " << msg << " *****\n";
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::read_settings(InputStreamEvents* ise)
{
    size_t result = 0;

    FileStream fs((get_config_path() + "config").c_str());
    while (size_t n = fs.read(ise))
    {
        result += n;
    }
    return 0;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::set_use_hardware_breakpoints(bool flag)
{
    uint64_t opts = options();
    if (flag)
    {
        opts |= OPT_HARDWARE_BREAKPOINTS;
    }
    else
    {
        opts &= ~OPT_HARDWARE_BREAKPOINTS;
    }
    set_options(opts);
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::init_frame_handlers(const Thread& thread)
{
    FrameHandlers::iterator i = frameHandlers_.begin();
    for (; i != frameHandlers_.end(); ++i)
    {
        (*i)->init(&thread);
    }
}


////////////////////////////////////////////////////////////////
RefPtr<Frame> DebuggerEngine::unwind_frame
(
    const Thread& thread,
    const Frame& frame
)
{
    struct Helper : public EnumCallback<Frame*>
    {
        RefPtr<Frame> frame_;
        virtual ~Helper() { }

        void notify(Frame* frame) { frame_ = frame; }
    } helper;

    FrameHandlers::iterator i = frameHandlers_.begin();
    for (; i != frameHandlers_.end(); ++i)
    {
        if ((*i)->unwind_step(&thread, &frame, &helper))
        {
            assert(helper.frame_.get());
            // do NOT break;
            // keep looping so that all handlers see the frame
        }
    }
    return helper.frame_;
}


////////////////////////////////////////////////////////////////
template<typename T>
static void print_version_info(ostream& outs, ImportPtr<T> p)
{
    if (VersionInfo* info = interface_cast<VersionInfo*>(p.get()))
    {
        cout << "*** " << info->description() << "  ";
        cout << info->copyright() << endl;
    }
}


/**
 * Make sure that the major version number of the plugin matches
 * the major version of the engine
 */
template<typename T> static bool
check_version(ImportPtr<T> p,
              const VersionInfo& vinfo,
              const string& fname)
{
    if (VersionInfo* info = interface_cast<VersionInfo*>(p.get()))
    {
        uint32_t minor = 0, pluginMinor = 0;
        uint32_t major = vinfo.version(&minor);

        if ((info->version(&pluginMinor) != major) || (pluginMinor > minor))
        {
            cout << basename(fname.c_str()) << " expects engine version ";
            cout << major << ".[" << pluginMinor << " or higher]\n";

            return false;
        }
    }
    return true;
}


static inline Priority::Class priority_class(Plugin* plugin)
{
    Priority::Class priorityClass = Priority::NORMAL;

    if (Priority* prio = interface_cast<Priority*>(plugin))
    {
        priorityClass = prio->priority_class();
    }
    return priorityClass;
}


////////////////////////////////////////////////////////////////
static void
register_plugin(DebuggerEngine::PluginList& list,
                ImportPtr<DebuggerPlugin>& plugin)
{
    switch (priority_class(plugin.get()))
    {
    case Priority::HIGH:
        list.push_front(plugin);
        break;

    default:
        if (!list.empty()
          && priority_class(list.back().get()) == Priority::LOW)
        {
            DebuggerEngine::PluginList::iterator i = list.end();
            --i;
            list.insert(i, plugin);

            assert(priority_class(list.back().get()) == Priority::LOW);
        }
        else
        {
            list.push_back(plugin);
        }
        break;
    }
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::on_interface(
    DynamicLibPtr   lib,
    uuidref_t       iid,
    Unknown2*&      component)
{
    if (uuid_equal(DebuggerPlugin::_uuid(), iid))
    {
        dbgout(0) << " detected plugin: " << lib->filename() << endl;
#if !defined(NDEBUG) || defined(DEBUG)
        const int count = lib->count();
#endif
        ImportPtr<DebuggerPlugin> plugin;
        create_instance(lib, plugin);
        if (!check_version(plugin, *this, lib->filename()))
        {
            return false;
        }
        assert(lib->count() >= count);
        component = plugin.get();
        assert(component);

        plugin->register_streamable_objects(interface_cast<ObjectFactory*>(this));
        register_plugin(plugins_, plugin);
        genericPlugins_.push_back(plugin);

        if ((options() & OPT_NO_BANNER) == 0)
        {
            print_version_info(cout, plugin);
        }
    }
    else if (uuid_equal(Disassembler::_uuid(), iid))
    {
        if (!disasm_.is_null())
        {
            clog << "*** Warning: " << lib->filename() << ": ";
            clog << " another disassembler is already loaded";

            if (const DynamicLib* module = disasm_.module())
            {
                clog << ": " << module->filename();
            }
            clog << endl;
            return false; // discard
        }
        else
        {
            create_instance(lib, disasm_);
            assert(!disasm_.is_null());
            if (!check_version(disasm_, *this, lib->filename()))
            {
                return false;
            }
            if ((options() & OPT_NO_BANNER) == 0)
            {
                print_version_info(cout, disasm_);
            }
            component = disasm_.get();
        }
    }
    else if (uuid_equal(FrameHandler::_uuid(), iid))
    {
        ImportPtr<FrameHandler> handler;
        create_instance(lib, handler);
        if (!handler || !check_version(handler, *this, lib->filename()))
        {
            return false;
        }

        //clog << "FrameHandler loaded: " << handler.get() << ": ";
        //clog << lib->filename() << endl;

        frameHandlers_.push_back(handler);
        component = handler.get();
    }
    else if (uuid_equal(DataFilter::_uuid(), iid))
    {
        if (!dataFilter_.is_null())
        {
            clog << "*** Warning: another data filter is loaded\n";
            return false;
        }
        else
        {
            create_instance(lib, dataFilter_);

            // create_instance post-condition, by contract
            assert(!dataFilter_.is_null());

            if (!check_version(dataFilter_, *this, lib->filename()))
            {
                return false;
            }
            if ((options() & OPT_NO_BANNER) == 0)
            {
                print_version_info(cout, dataFilter_);
            }
            component = dataFilter_.get();
        }
    }
    else if (uuid_equal(GenericPlugin::_uuid(), iid))
    {
        ImportPtr<GenericPlugin> plugin;
        create_instance(lib, plugin);
        genericPlugins_.push_back(plugin);

        component = plugin.get();
    }

    return PluginManager::on_interface(lib, iid, component);
}



////////////////////////////////////////////////////////////////
static bool init_plugin (
    ImportPtr<GenericPlugin>&   plugin,
    DebuggerEngine*             engine,
    int&                        argc,
    char**&                     argv,
    set<string>&                errs)
{
    bool result = false;

    try
    {
        result = plugin->initialize(engine, &argc, &argv);
    }
    catch (const std::exception& e)
    {
        errs.insert(e.what());
    }
    catch (...)
    {
        errs.insert("Unknown error while initializing plugin");
    }
    return result;
}


////////////////////////////////////////////////////////////////
static void start_plugin(
    ImportPtr<GenericPlugin>&   plugin,
    set<string>&                errs)
{
    try
    {
        plugin->start();
    }
    catch (const std::exception& e)
    {
        errs.insert(e.what());
    }
    catch (...)
    {
        errs.insert("Unknown error while starting plugin");
    }
}


////////////////////////////////////////////////////////////////
//
// After the scan is complete, procede to initialize plugins
//
void DebuggerEngine::on_scan_plugins_complete()
{
    set<string> err;

    GenericPlugins::iterator i = genericPlugins_.begin();
    for (; i != genericPlugins_.end(); )
    {
        // pass the command line arguments to plug-ins
        if (init_plugin(*i, this, argc_, argv_, err))
        {
            ++i;
        }
        else
        {
            PluginList::iterator j = find(plugins_.begin(), plugins_.end(), *i);
            if (j != plugins_.end())
            {
                dbgout(0) << "discarding " << j->module()->filename() << endl;
                plugins_.erase(j);
            }
            i = genericPlugins_.erase(i);
        }
    }
    unload_unreferenced();
    check_unknown_cmdline_opts();

    for (i = genericPlugins_.begin(); i != genericPlugins_.end(); ++i)
    {
        start_plugin(*i, err);
    }

    add_plugin_custom_commands();
    pluginsInitialized_ = true;

    while (!err.empty())
    {
        critical_error(NULL, err.begin()->c_str());
        err.erase(err.begin());
    }

    if (quitCommandPending_)
    {
        quit();
    }
}



////////////////////////////////////////////////////////////////
void DebuggerEngine::add_plugin_custom_commands()
{
    PluginList::iterator i = plugins_.begin();

    for (i = plugins_.begin(); i != plugins_.end(); ++i)
    {
        DebuggerPlugin* p = i->get();

        if (CommandCenter* cc = interface_cast<CommandCenter*>(p))
        {
            CommandList::iterator j = addons_.begin();
            for (; j != addons_.end(); ++j)
            {
                cc->add_command(j->get());
            }
        }
    }
    addons_.clear();
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::check_unknown_cmdline_opts()
{
    for (int argc = 1; argc < argc_; ++argc)
    {
        const char* arg = argv_[argc];
        if (*arg != '-')
        {
            continue;
        }
        if (strcmp(arg, "--") == 0)
        {
            break;
        }
        else
        {
            cerr << "*** Option not understood: " << arg << endl;
        }
    }
}


namespace
{
    template<typename T>
    struct ZDK_LOCAL UnitLookup_Match
    {
        TranslationUnit*
            operator()(Process* proc,
                       DebugInfoReader* r,
                       SharedString* module,
                       T arg) const
        {
            return r->lookup_unit_by_addr(proc, module, arg);
        }
    };

    /**
     * Specialization for filename lookups
     */
    template<>
    class ZDK_LOCAL UnitLookup_Match<const char*>
        : EnumCallback<TranslationUnit*, bool>
    {
        RefPtr<TranslationUnit> unit_;

    public:
        TranslationUnit* operator()
            (
                Process* process,
                DebugInfoReader* r,
                SharedString* module,
                const char* fname
            )
        {
            r->lookup_unit_by_name(process, module, fname, this);
            return unit_.detach();
        }

        bool notify(TranslationUnit* unit)
        {
            unit_ = unit;
            return unit_.is_null();
        }
    };


    template<typename T>
    class ZDK_LOCAL UnitLookupHelper : public EnumCallback<Module*>
    {
        typedef DebuggerEngine::PluginList PluginList;

        UnitLookup_Match<T>     match_;
        RefPtr<Process>         proc_;
        const T                 arg_;
        const PluginList&       plugins_;

    public:
        TranslationUnit* unit_;

        UnitLookupHelper(Process* proc, T arg, const PluginList& plugins)
            : proc_(proc), arg_(arg), plugins_(plugins), unit_(0)
        { }

        void notify(Module* module)
        {
            if (unit_ || !module)
            {
                return;
            }
            SharedString* name = module->name();
            if (!name)
            {
                return;
            }
            PluginList::const_iterator i = plugins_.begin();
            const PluginList::const_iterator end = plugins_.end();

            for (; i != end && !unit_; ++i)
            {
                if (DebugInfoReader* r =
                    interface_cast<DebugInfoReader*>(i->get()))
                {
                    unit_ = match_(proc_.get(), r, name, arg_);
                }
            }
        }
    };
} // namespace


////////////////////////////////////////////////////////////////
TranslationUnit* DebuggerEngine::lookup_unit_by_addr(
    Process* proc,
    addr_t addr
    ) const
{
    UnitLookupHelper<addr_t> helper(proc, addr, plugins_);
    enum_modules(&helper);
    return helper.unit_;
}


////////////////////////////////////////////////////////////////
TranslationUnit* DebuggerEngine::lookup_unit_by_name(
    Process* proc,
    const char* name
    ) const
{
    UnitLookupHelper<const char*> helper(proc, name, plugins_);
    enum_modules(&helper);

    return helper.unit_;
}



////////////////////////////////////////////////////////////////
ZObject* DebuggerEngine::current_action_context() const
{
    ZObject* context = NULL;
    if (!actionContextStack_.empty())
    {
        context = actionContextStack_.top().get();
    }
    //dbgout(1) << __func__ << ": " << context << endl;
    return context;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::push_action_context (ZObject* obj)
{
    actionContextStack_.push(obj);
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::pop_action_context()
{
    if (actionContextStack_.empty())
    {
        throw logic_error("pop_action_context: stack empty");
    }
    actionContextStack_.pop();
}


////////////////////////////////////////////////////////////////
uint32_t DebuggerEngine::version(uint32_t* minor, uint32_t* rev) const
{
    if (minor)
    {
        *minor = ENGINE_MINOR;
    }
    if (rev)
    {
        *rev = ENGINE_REVISION;
    }
    return ENGINE_MAJOR;
}


////////////////////////////////////////////////////////////////
const char* DebuggerEngine::description() const
{
#if DEBUG
    return "Debugger Engine BETA (" __DATE__ ")";
#else
    return "Debugger Engine (" __DATE__ ")";
#endif
}


////////////////////////////////////////////////////////////////
const char* DebuggerEngine::copyright() const
{
    return copyrightText;
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::add_command(DebuggerCommand* cmd)
{
    dbgout(0) << __func__ << ": " << cmd->name() << endl;
    addons_.push_back(cmd);

    // if not all plugins are initialized, delay adding the
    // custom command (so that we do not miss a plugin that
    // may be interested in seeing the custom command, but has
    // not been loaded and initialized yet)
    //
    if (pluginsInitialized_)
    {
        add_plugin_custom_commands();
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::enable_command(DebuggerCommand* cmd, bool enable)
{
    PluginList::iterator i = plugins_.begin();

    for (i = plugins_.begin(); i != plugins_.end(); ++i)
    {
        DebuggerPlugin* p = i->get();

        if (CommandCenter* cc = interface_cast<CommandCenter*>(p))
        {
            cc->enable_command(cmd, enable);
        }
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::set_breakpoint_at_catch(Thread* thread)
{
    // todo: support other compilers than GCC,
    static const SymbolTable::LookupMode mode =
        SymbolTable::LKUP_DYNAMIC   |
        SymbolTable::LKUP_UNMAPPED  |
        SymbolTable::LKUP_ISMANGLED;

    FunctionEnum funcs;

    if (thread->symbols()->enum_symbols("__cxa_begin_catch", &funcs, mode))
    {
        static const word_t cookie = 1;

        RefPtr<BreakPointAction> act(new EngineAction(
            *this, "__catch", &DebuggerEngine::break_at_catch, true, cookie));

        set_breakpoint(*thread, funcs, act.get());
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::set_breakpoint_at_throw(
    Thread* thread,
    bool permanent)
{
    // todo: should the name of the throw func be configurable?

    static const char* throwFunc[] = {
        "__cxa_throw",  // Itanium-ABI conforming C++ compilers
        "__throw",      // GCC 2.95
        "_d_throw@4",   // Digital Mars D
    };
    // A good use for temporary breakpoints at __throw is in
    // the expression interpreter which sets one before calling
    // a function, and removes it upon returning from the call.

    const char* actionName = permanent ? "__throw" : "__throw_once";
    static const word_t cookie = 1;

    RefPtr<BreakPointAction> action(new EngineAction(
            *this,
            actionName,
            &DebuggerEngine::break_at_throw,
            permanent,
            cookie));
    
    FunctionEnum funcs;

    for (size_t i = 0; i != sizeof(throwFunc)/sizeof(throwFunc[0]); ++i)
    {
        static const SymbolTable::LookupMode mode =
            SymbolTable::LKUP_DYNAMIC   |
            SymbolTable::LKUP_UNMAPPED  |
            SymbolTable::LKUP_ISMANGLED;

        thread->symbols()->enum_symbols(throwFunc[i], &funcs, mode);
        set_breakpoint(*thread, funcs, action.get());
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::set_breakpoint(

    Thread&             thread,
    const SymbolEnum&   funcs,
    BreakPoint::Action* action)
{
    if (!funcs.empty())
    {
        Runnable* runnable = get_runnable(&thread);

        for (auto i = funcs.begin(); i != funcs.end(); ++i)
        {
            Symbol* sym = i->get();

            ZObjectScope scope;
            SymbolTable* table = sym->table(&scope);
            if (!table)
            {
                continue;
            }

            if (table->addr()) // mapped into memory?
            {
                CHKPTR(breakpoint_manager())->set_breakpoint(
                    runnable, BreakPoint::GLOBAL, sym->addr(), action);

                dbgout(0) << __func__ << ": " << (void*)sym->addr() <<endl;
            }
            else
            {
                dbgout(0) << __func__ << ": deferred brkpnt in "
                          << table->filename() << endl;
            #if 0
                // this may create problems if the target is an exec-ed process
                //NULL means "all threads"
                sym->set_deferred_breakpoint(BreakPoint::GLOBAL, NULL, action);
            #else
                sym->set_deferred_breakpoint(BreakPoint::GLOBAL, get_runnable(&thread), action);
            #endif
            }
        }
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::set_options(uint64_t opts)
{
    if (opts != options())
    {
        DebuggerBase::set_options(opts);
        assert(options() == opts);

        if (opts & OPT_BREAK_ON_THROW)
        {
            if (Thread* thread = get_thread(DEFAULT_THREAD))
            {
                if (thread->is_live())
                {
                    set_breakpoint_at_throw(thread);
                }
            }
        }
        else
        {
            remove_breakpoint_action(0, 0, 0, "__throw");
        }
    }
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::activate_deferred_breakpoint (
    BreakPoint* bpnt,
    const SymbolTable& symTab)
{
    if (breakpoint_manager())
    {
        breakPointMgr_->activate(bpnt, symTab);
    }
}



////////////////////////////////////////////////////////////////
void DebuggerEngine::add_step_over(SharedString* file, long lineNum)
{
    stepOverMgr_->add_step_over(file, lineNum);
}



////////////////////////////////////////////////////////////////
void DebuggerEngine::remove_step_over(SharedString* file, long lineNum)
{
    stepOverMgr_->remove_step_over(file, lineNum);
}


////////////////////////////////////////////////////////////////
size_t DebuggerEngine::enum_step_over(
    EnumCallback2<SharedString*, long>* cb) const
{
    return stepOverMgr_->enum_step_over(cb);
}


////////////////////////////////////////////////////////////////
bool DebuggerEngine::query_step_over(
    const SymbolMap* symbols, addr_t pc) const
{
    return stepOverMgr_->query_step_over(symbols, pc);
}


////////////////////////////////////////////////////////////////
void DebuggerEngine::restore_step_over_properties()
{
    stepOverMgr_->restore_from_properties();
}


////////////////////////////////////////////////////////////////
RefPtr<BreakPointAction>
DebuggerEngine::interactive_action(const char* name, bool permanent)
{
    return new EngineAction(
            *this,
            name,
            &DebuggerEngine::break_into_interactive_mode,
            permanent);
}



////////////////////////////////////////////////////////////////
size_t DebuggerEngine::enum_tables_by_source (
    SharedString* fname,
    EnumCallback<SymbolTable*>* callback
    ) const
{
    throw logic_error(__func__ + string(": API not implemented"));
/*
    if (symbolEvents_.get())
    {
        return symbolEvents_->enum_tables_by_source(fname, callback);
    }
 */
    return 0;
}
// Copyright (c) 2004, 2006, 2007 Cristian L. Vlasceanu
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

