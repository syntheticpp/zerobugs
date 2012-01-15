#ifndef DEBUGGER_BASE_H__F15DCC88_2448_4BAC_B777_91B645682570
#define DEBUGGER_BASE_H__F15DCC88_2448_4BAC_B777_91B645682570
//
// $Id: debugger_base.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sys/types.h>  // pid_t
#include <signal.h>
#include <iosfwd>
#include <queue>
#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "zdk/mutex.h"
#include "zdk/stream.h"
#include "zdk/zero.h"
#include "dbgout.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "dharma/sarray.h"
#include "process.h"
#include "unhandled_map.h"
#include "target/target.h"
#include "target/target_manager.h"


class ExecArg;
struct HistoryEntry;
class HistoryEntryImpl;
class SignalPolicyImpl;
class Settings;
class SymbolTableEvents;
class TargetFactory;
class ThreadImpl;


std::ostream& operator<<(std::ostream&, const SignalPolicy&);


/**
 * The base debugger deals with coordinating the debugged
 * (traced) threads. By making functions such as wait_for_event
 * private to the base class, we make sure that the upper
 * levels of the debugger don't mess with the thread management.
 *
 * see zdk/zero.h for the Debugger interface definition
 */
class DebuggerBase : public Debugger
                   , public TargetManager
                   , public std::ios::Init
{
    // non-copyable, non-assignable
    DebuggerBase(const DebuggerBase&);
    DebuggerBase& operator=(const DebuggerBase&);

protected:
BEGIN_INTERFACE_MAP(DebuggerBase)
    INTERFACE_ENTRY(Debugger)
    INTERFACE_ENTRY_DELEGATE(factory_)
    INTERFACE_ENTRY_DELEGATE(unhandled_)
END_INTERFACE_MAP()

    typedef boost::shared_ptr<SignalPolicyImpl> SignalPolicyPtr;

    /* for queuing debugged threads that have events pending */
    typedef std::queue<RefPtr<ThreadImpl> > ThreadQueue;
    typedef std::vector<SignalPolicyPtr> SignalPolicyList;

    /* for map_path */
    typedef std::map<std::string, std::string> PathMap;


    DebuggerBase();

    virtual ~DebuggerBase() throw();

    /**
     * Execute program.
     * @param args a tokenized command line.
     * @param shellExpandArgs
     * @param environ optionally pass in environment.
     */
    pid_t exec(const ExecArg& args,
               bool shellExpandArgs,
               const char* const* env);

    const char* const* environment(bool reset = false);

    void set_environment(const char* const*);

    void print_counted_objects(const char*) const;

    /**
     * helper function called by attach_or_exec
     */
    bool try_attach(std::string&, size_t, const char* = NULL);

    void read_process_history(Process&);

    RefPtr<HistoryEntryImpl> get_history_entry(Process&) const;

    /**
     * Enable/disable breakpoints
     * @param onOff if true, enable; otherwise disable
     * @param flags what type of breakpoints are affected
     */
    void enable_breakpoints(
        bool onOff,
        int flags = (BreakPoint::SOFTWARE | BreakPoint::EMULATED));

public:
    size_t enum_user_tasks(EnumCallback<const Runnable*>*,
                           const char* targetParam = NULL);

    void attach_or_exec(const ExecArg&);

    void load_core(const char* core, const char* program);

    bool has_corefile() const;

    /**
     * Attaches debugger to a process.
     */
    void attach(pid_t, const char* = NULL);

    /**
     * Detach from current process (and all of its threads)
     */
    void detach();

    /**
     * @return true if successful
     */
    bool detach(const std::nothrow_t&);

    /**
     * Execute a program from within the debugger, and attach to it.
     * @note the string can contain command line arguments for the
     * debuggee, separated by blanks.
     */
    pid_t exec(const char*, bool = false, const char* const* = 0);

    /**
     * Send a SIGSTOP to the debugged program, and return
     * without waiting for the threads to stop.
     */
    void stop();

    /**
     * Returns the verbosity level. The verbosity is used for
     * displaying messages to the console. Such messages are
     * useful for debugging the debugger.
     */
    int verbose() const { return verbose_; }

    void set_verbose(int v) { verbose_ = v; }

    uint64_t options() const;

    void set_options(uint64_t);

    /**
     * The debugger event loop.
     */
    void run();

    virtual void quit();

    virtual size_t enum_processes(EnumCallback<Process*>*) const;

    virtual size_t enum_threads(EnumCallback<Thread*>*);

    virtual Thread* get_thread(pid_t, unsigned long = 0) const;

    /**
     * Get the signal handling policy for sigNum
     */
    virtual SignalPolicy* signal_policy(int sigNum);

    virtual Properties* properties();

    void refresh_properties();

    /**
     * Called after a thread has updated its own state.
     * The debugger currently uses this notification to
     * remove finished threads from its internal data.
     */
    void on_update(Thread&);

    /**
     * Called when a thread is signaled, expected
     * to return immediately; the debugger queues
     * the event, which is going to be processed
     * when control returns to the main event loop.
     */
    void on_resuming(Thread&);

    /**
     * Called after the entire debugged process
     * (all threads) have resumed execution
     */
    virtual void on_resumed();

    /**
     * @return a custom implementation of the SymbolTableEvents
     * interface, that gets called during reading symbol tables.
     */
    virtual SymbolTableEvents* symbol_table_events();

    /**
     * restore breakpoints and other settings for this
     * module (if history entry available)
     */
    bool restore_module(Process&,
                        Module&,
                        RefPtr<HistoryEntryImpl> = NULL);

    bool is_attached() const;

    void set_default_signal_policies();

    void set_lwpid_step(pid_t pid) { lwpidStep_ = pid; }

    pid_t lwpid_step() const { return lwpidStep_; }

    virtual void cleanup(Thread&);

    virtual void queue_event(const RefPtr<ThreadImpl>&);

    /**
     * A debugged thread received a signal or has exited
     */
    virtual void on_event(Thread&);

    /**
     * Called when attached to a new thread
     */
    virtual void on_attach(Thread&);

    bool trace_fork() const { return (options() & OPT_TRACE_FORK) != 0; }

    virtual void critical_error(Thread*, const char*) { }

    void save_lwpid_and_status(pid_t pid, int status)
    {
        if (unhandled_)
        {
            unhandled_->add_status(pid, status);
        }
    }

    UnhandledMap* unhandled_map() { return unhandled_.get(); }

    virtual bool map_path(const Process*, std::string&) const;

    DebugChannel debug_channel(const char* fn) const
    {
        return DebugChannel(fn, verbose_);
    }

    bool initial_thread_fork() const { return initialThreadFork_; }
    void set_initial_thread_fork(bool f) { initialThreadFork_ = f; }

    static TargetFactory& target_factory();

protected:
    void stop_all_threads(Thread* = NULL);

    /**
     * Called when there are no attached targets
     */
    virtual void on_idle();

    static void print_event_info(std::ostream&, const Thread&);

    /**
     * @return queued thread (from the front of the queue), or NULL
     */
    RefPtr<ThreadImpl> peek_event(bool remove);

    /**
     * this flag indicates that either:
     * 1) the debugge has been launched from the command line, or
     * 2) a process has been exec-ed
     */
    bool have_new_program() const { return haveNewProgram_; }
    void set_have_new_program(bool flag) { haveNewProgram_ = flag; }

    void detach_all_targets();

    void resume(bool = true) { }

    bool is_quitting() const { return quit_; }

    void take_snapshot();

    const HistoryEntry* get_most_recent_history_entry();

    virtual size_t enum_modules(EnumCallback<Module*>*) const;

    void save_properties();

    void clear_properties();

    void reset_properties();

    void set_trace_fork(bool flag)
    {
        if (flag)
        {
            options_ |= OPT_TRACE_FORK;
        }
        else
        {
            options_ &= ~OPT_TRACE_FORK;
        }
    }

    bool is_silent() const { return (options() & OPT_SILENT) != 0; }

    void set_silent(bool silent)
    {
        if (silent)
        {
            options_ |= OPT_SILENT;
        }
        else
        {
            options_ &= ~OPT_SILENT;
        }
    }

    void detach_internal();

    void throw_signal_out_of_range(const char* func, int);

    static std::string get_config_path();

    void set_signaled(bool f) { signaled_ = f; }
    bool signaled() const { return signaled_;  }

private:
    void resume_threads();

    void check_unhandled_events();

    /**
     * map path without following symbolic links.
     */
    bool map_path_no_follow(const Process*, std::string&) const;

    /**
     * Waits for one of the traced threads to stop in the
     * debugger (because of a signal) and return it.
     */
    RefPtr<Thread> get_event();

    void enter() { TargetManager::mutex().enter(); }
    void leave() { TargetManager::mutex().leave(); }

private:
    ThreadQueue             queue_;     // queued thread debug events
    SignalPolicyList        signalPolicies_;

    RefPtr<Settings>        settings_;
    std::string             settingsPath_;
    mutable Mutex           settingsMutex_;

    // verbosity level for printing debug messages to console
    mutable int             verbose_;

    // environment passed to debugged progs
    SArray                  env_;
    bool                    haveNewProgram_;
    bool                    quit_;
    bool                    signaled_;
    bool                    initialThreadFork_;
    bool                    historySnapshotsEnabled_;
    uint64_t                options_;
    pid_t                   lwpidStep_; // last thread to single-step
    RefPtr<UnhandledMap>    unhandled_; // for out-of-order events
    RefPtr<ObjectFactory>   factory_;
    time_t                  startupTime_;
    mutable PathMap         pathMap_;
};



#endif // DEBUGGER_BASE_H__F15DCC88_2448_4BAC_B777_91B645682570
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
