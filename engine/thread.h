#ifndef THREAD_H__E08DF5DC_BE68_4089_AC18_2F3EB68753D4
#define THREAD_H__E08DF5DC_BE68_4089_AC18_2F3EB68753D4
//
// $Id: thread.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Implementations of Runnable and Thread interfaces
//
#ifdef HAVE_SYS_PARAM_H
 #include <sys/param.h>
#endif
#include <sys/types.h>          // pid_t
#ifdef HAVE_SYS_USER_H
 #include <sys/user.h>          // user_regs_struct
#endif
#include <iosfwd>               // std::ostream
#include <memory>
#include <boost/utility.hpp>    // noncopyable
#include "zdk/config.h"
#include "zdk/ref_ptr.h"
#include "zdk/weak_ptr.h"
#include "dharma/object_manager.h"
#include "debug_regs_base.h"
#include "runnable_state.h"
#include "thread_base.h"

#ifdef HAVE_THREAD_DB_H
extern "C"
{
 #include <thread_db.h>         // for thread_t
}
#elif !defined(HAVE_THREAD_T)
 typedef long thread_t;
#endif


class SymbolTableEvents;
class StackTraceImpl;
class ThreadImpl;

typedef RefPtr<SymbolMap> SymbolMapPtr;
typedef RefPtr<StackTraceImpl> StackTracePtr;


/**
 * Encapsulates some basic lightweight process information.
 *
 * @note copying a Runnable does not make sense --
 * unless we actually clone()-it -- so this class
 * is non-copyable.
 */
class RunnableImpl : public Runnable
                   , public RunnableState
                   , boost::noncopyable
{
public:
    DECLARE_UUID("2f34935c-410d-4e70-a69f-230a791ba6de")

BEGIN_INTERFACE_MAP(RunnableImpl)
    INTERFACE_ENTRY(Runnable)
    INTERFACE_ENTRY(RunnableImpl)
END_INTERFACE_MAP()

    RunnableImpl(pid_t lwpid, Target*, ThreadImpl* = 0);
    virtual ~RunnableImpl() throw() {}

    virtual pid_t pid() const { return lwpid_; }
    virtual pid_t ppid() const { return ppid_; }
    virtual pid_t gid() const { return gid_; }

    virtual const char* name() const;

    virtual Process* process() const;
    virtual Thread* thread() const;

    /**
     * @return the running state of the process
     */
    virtual State runstate() const;

    virtual size_t vmem_size() const { return vmemSize_; }

    /**
     * The number of clock ticks that this process
     * has been scheduled in user mode so far.
     * @see man clock(3)
     */
    virtual size_t usr_ticks() const { return usrTicks_; }

    /**
     * The number of clock ticks that this process
     * has been scheduled in system mode.
     * @see man clock(3)
     */
    virtual size_t sys_ticks() const { return sysTicks_; }

    virtual addr_t stack_start() const;

    virtual size_t enum_open_files(EnumCallback2<int, const char*>*) const;

    virtual void set_result(word_t);

    virtual void set_result64(int64_t);

    virtual void set_result_double(long double, size_t);

    virtual void set_program_count(addr_t);

    virtual void set_stack_pointer(addr_t);

    virtual void write_register(size_t, reg_t);
    virtual bool write_register(Register*, const Variant*);

    virtual void set_registers(ZObject*, ZObject*);

    /**
     * Set the single stepping mode of this thread,
     * optionally specifying an action context. The
     * action context may be used later to identify
     * the entity that initiated the single-stepping.
     */
    virtual void set_single_step_mode(bool, ZObject* = NULL);

    /**
     * Execute one single machine instruction
     */
    virtual void step_instruction();

    /**
     * Execute instructions until the current function returns.
     * @note the method is asynchronous; the caller should not
     * expect the debugged function to return when this method
     * completes; rather, the method arranges for the thread to
     * break in the debugger when the current function returns.
     * @note it is up to the implementation if this is achieved
     * by single-stepping or by setting a temporary, internal
     * breakpoint at the return address on the stack.
     */
    virtual void step_until_current_func_returns();

    /**
     * @todo
     */
    virtual void freeze(bool);

    /**
     * @todo
     */
    virtual bool is_frozen() const;

    // virtual bool is_exiting() const { return false; }

    friend std::istream& operator>>(std::istream&, RunnableImpl&);

    /**
     * read the state from /proc/<pid>/stat
     * @todo the implementation is Linux-specific and
     * should be delegated to Target implementations
     */
    void refresh();

    RefPtr<Target> target() const { return target_.ref_ptr(); }

    void set_single_step(bool mode) { singleStep_ = mode; }
    bool single_step() const { return singleStep_; }

    void clear_cached_regs();

    bool resume();

    void set_next(Symbol*);
    void set_stepping(bool);

private:
    mutable RefPtr<SharedString> name_;

    WeakPtr<Target> target_;
    ThreadImpl* thread_;
    bool singleStep_;
};



/**
 * Model of a thread in a running, debugged program (AKA LIVE THREAD).
 * @note "running program" is used here to distinguish from a core dump debug target.
 */
class ThreadImpl : public ThreadBase
{
public:
    // Forward decl of a class that describes a
    // range of addresses to single-step through.
    class StepRange;

    class DebugChannel;

    typedef std::auto_ptr<StepRange> StepRangePtr;

    DECLARE_UUID("2ee14715-4ecd-4004-8ed4-4ec6c5387002")

    BEGIN_INTERFACE_MAP(ThreadImpl)
        INTERFACE_ENTRY(ThreadImpl)
        INTERFACE_ENTRY(Thread)
        INTERFACE_ENTRY_DELEGATE(&runnable_)
        INTERFACE_ENTRY_DELEGATE(process())
    END_INTERFACE_MAP()

    ThreadImpl(Target&, thread_t, pid_t);

    virtual ~ThreadImpl() throw();

    virtual Target* target() const
    { return runnable_.target().get(); }

    virtual pid_t lwpid() const;

    virtual pid_t ppid() const { return runnable_.ppid(); }

    virtual pid_t gid() const { return runnable_.gid(); }

    virtual unsigned long thread_id() const;

    void set_thread_id(thread_t id);

    virtual Runnable::State runstate() const
    { return runnable_.runstate(); }

    virtual const char* filename() const;

    virtual bool is_live() const { return true; }

    virtual bool is_forked() const { return forked_; }
    virtual bool is_execed() const { return execed_; }

    void set_forked();

    void set_execed() { assert(!execed_); execed_ = true; }

    virtual addr_t stack_start() const
    { return runnable_.stack_start(); }

    virtual addr_t ret_addr();

    virtual int status() const { return status_; }

    void set_status(int status);

    virtual int signal() const;

    void set_signal(int);

    /**
     * update the status without throwing
     * @return true on success, false if an error occurred
     */
    bool update(const std::nothrow_t&) throw();

    SymbolMap* symbols() const;

    void read_symbols(SymbolTableEvents*);

    size_t enum_cpu_regs(EnumCallback<Register*>*);

    StackTrace* stack_trace(size_t = INT_MAX) const;

    size_t stack_trace_depth() const;

    void notify_resuming();

    void notify_queue();

    void set_event_pending(bool);

    bool is_event_pending() const;

    bool single_step_mode() const;

    void step_instruction();

    void step_until_current_func_returns();

    DebugSymbol* func_return_value();

    /**
     * @return true whether stepping should dive
     * into function calls, false if should step over
     */
    bool step_into() const;

    void set_step_into(bool flag);

    /**
     * Initiate single-stepping thru the given range of addresses
     * @param context identifies the initiator object; may be NULL
     */
    void step_thru(addr_t first, addr_t last, ZObject* context);

    /**
     * was the thread intentionally stopped by us?
     */
    bool is_stopped_by_debugger() const;

    /**
     * Mark thread as intentionally stopped by debugger.
     */
    void set_stopped_by_debugger(bool flag)
    {
        debuggerStop_ = flag;
    }

    virtual void set_user_data(const char*, word_t, bool);

    virtual bool get_user_data(const char*, word_t*) const;

    void set_user_object(const char*, ZObject*, bool = true);

    ZObject* get_user_object(const char*) const;

    void detach();

    void wait_update_status(bool checkUnhandledThreads = false);

    void set_call_info(addr_t caller = 0);

    void check_return_value();

    bool is_addr_in_step_range(addr_t, bool closedEnd = false) const;
    //bool is_stepped_over(addr_t) const;

    bool is_line_stepping() const;

    pid_t wait_internal(int*, int, bool no_throw = false);

    bool resume(bool* stepped = NULL);

    bool has_resumed() const { return resumed_; }

    void set_exiting() { exiting_ = true; }

    bool is_exiting() const { return exiting_; }

    bool is_done_stepping() const;

    void set_done_stepping(bool f) { finishedStepping_ = f; }

    bool is_stepping_to_return() const { return stepReturn_; }

    void set_stepping_to_return(bool f) { stepReturn_ = f; }

    virtual ZObject* action_context() const
    { return actionContext_.get(); }

    void set_action_context(ZObject* object)
    { actionContext_.reset(object); }

    void refresh() { runnable_.refresh(); }

    void reset_stack_trace();

    pid_t get_signal_sender() const;

    void set_stop_expected(bool f = true) { expectStop_ = f; }
    bool is_stop_expected() const { return expectStop_; }

    void dump_debug_regs(std::ostream& out) const
    { assert(debugRegs_.get()); debugRegs_->dump(out); }

    Runnable& runnable() { return runnable_; }

    /**
     * @return the destination Symbol for stepping over a line of source code,
     * without stepping into function calls
     */
    RefPtr<Symbol> get_next() const { return next_; }
    void set_next(Symbol*);

    /**
     * @return true if okay to step into function calls
     */
    bool is_stepping() const;
    void set_stepping(bool);

    bool exited(int* status) const;

private:
    friend void RunnableImpl::set_single_step_mode(bool, ZObject*);

    void set_single_step_mode(bool, ZObject* context);
    void set_step_over_breakpoint(addr_t);

    void before_resume();

    /**
     * Called when we're about to resume the execution
     * of a thread stopped in the debugger; returns true
     * if the thread is actually resumed, false if
     * the caller needs to resume the thread.
     */
    bool resume_stepping();

    /**
     * prevent client code from directly calling this method
     * (lwpid should be called instead)
     */
    pid_t pid() const { return runnable_.pid(); }

    void ptrace_wrapper(int);

    void set_status_internal(int);

    void single_step_now();

    /**
     * Check for a new thread being fork-ed or cloned
     */
    void check_thread_creation_events();

    DebugChannel debug_channel(const char* fun) const;

    DebugRegs* debug_regs();

    // helper called from resume_stepping
    bool stepped_recursive(reg_t pc);

private:
    mutable thread_t threadID_;

    SymbolMapPtr    symbols_;           // todo: use target_?
    Debugger*       debugger_;
    int             status_;            // as reported by waitpid()
    int             eventPending_;
    pid_t           sender_;            // last signal sender pid
    union
    {
        int         flags_;
        struct
        {
            bool    singleStepMode_ : 1;
            bool    debuggerStop_ : 1;
            bool    forked_ : 1;
            bool    execed_ : 1;
            bool    resumed_ : 1;
            bool    exiting_ : 1;
            bool    deleted_ : 1;
            bool    finishedStepping_ : 1;
            bool    stepReturn_ : 1;
            bool    expectStop_ : 1;
            bool    isStepping_ : 1;
        };
    };
    StepRangePtr    stepRange_;

    mutable StackTracePtr oldTrace_;    // old stack trace
    mutable StackTracePtr trace_;       // current stack trace
    mutable Mutex   mutex_;

    RefPtr<Symbol>  called_;            // for function return value
    addr_t          callerProgramCount_;// ditto

    RefPtr<ZObject> actionContext_;
    RunnableImpl    runnable_;

    RefPtr<DebugSymbol> retVal_;
    std::auto_ptr<DebugRegsBase> debugRegs_;

    RefPtr<Symbol>  next_;
    size_t          nextStackDepth_;    // stack trace depth when
                                        // beginning to step over
};


std::ostream& operator<<(std::ostream&, const Thread&);

/**
 * wrapper function, if the actual program counter is at
 * a system call, return the caller's return addr on the stack
 */
ZDK_LOCAL addr_t program_count(const Thread&);


#endif // THREAD_H__E08DF5DC_BE68_4089_AC18_2F3EB68753D4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
