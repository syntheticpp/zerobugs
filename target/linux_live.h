#ifndef LINUX_LIVE_H__0266AD83_58A4_11DA_B82B_00C04F09BBCC
#define LINUX_LIVE_H__0266AD83_58A4_11DA_B82B_00C04F09BBCC
//
// $Id: linux_live.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "linux.h"
#include "linker_events.h"
#include "thread_events.h"

class ThreadImpl;


/**
 * Bridge between LinuxTarget objects and thread_db library.
 */
struct ps_prochandle : public LinuxTarget
{
    RefPtr<SymbolTable> symtab_;

protected:
    ps_prochandle(debugger_type& dbg) : LinuxTarget(dbg) { }
};

typedef ThreadEventObserver<ps_prochandle> LinuxLiveBase;


/**
 * Debugging target: a live linux process
 * @note models targets executed from under the debugger,
 * AND running targets the debugger attaches to.
 */
class LinuxLiveTarget : public LinkerEventObserver<LinuxLiveBase>
{
protected:
    typedef LinkerEventObserver<LinuxLiveBase> Super;
    typedef std::vector<RefPtr<Thread> > ThreadList;
    typedef ext::hash_map<pid_t, RefPtr<ThreadImpl> > ThreadImplMap;

    DECLARE_VISITABLE()

    Kind kind() const;

    word_t get_breakpoint_opcode(word_t word) const;
    addr_t stack_reserve(Thread&, addr_t) const;
    addr_t stack_align(Thread&, addr_t) const;

    Thread* exec(const ExecArg&, const char* const* env);

    virtual void step_until_safe(Thread&, addr_t) const;

    // MemoryIO
    virtual void read_memory(
        pid_t,
        SegmentType,
        addr_t,
        word_t*,
        size_t,
        size_t*) const;

    virtual void write_memory(
        pid_t,
        SegmentType,
        addr_t,
        const word_t*,
        size_t);

    // register access
    virtual bool read_register(const Thread&, int, bool, reg_t&) const;
    virtual void write_register(Thread&, size_t n, reg_t);

public:
    LinuxLiveTarget(debugger_type&, unsigned wordSize = __WORDSIZE);
    ~LinuxLiveTarget() throw();

    virtual void detach(bool no_throw = false);

    /**
     * Stop all threads in the target, optionally
     * passing in the thread that received an event.
     */
    virtual bool stop_all_threads(Thread* = NULL);

    virtual void stop_async();

    virtual size_t resume_all_threads();

    virtual size_t cleanup(Thread&);

    virtual bool event_requires_stop(Thread*);

    virtual void handle_event(Thread*);

    /**
     * Override ThreadAgentWrapper implementation: if a new thread,
     * add it to the internal collection of threads
     */
    void on_thread(const td_thrhandle_t*, const td_thrinfo&);

    pid_t pid_to_lwpid(pid_t pid) const { return pid; }

    RefPtr<SymbolMap> read_symbols();

    RefPtr<ProcessImpl> new_process(pid_t, const ExecArg*, ProcessOrigin);

    RefPtr<Thread> new_thread(long id, pid_t lwpid, int status);

    void attach(pid_t);

    ZObject* regs(const Thread&) const;

    ZObject* fpu_regs(const Thread&) const;

    bool write_register(Register&, const Variant&);

    void read_environment(SArray&) const;

    bool read_state(const Thread&, RunnableState&) const;

    const std::string& command_line() const;

    bool pass_by_reg(Thread&, std::vector<RefPtr<Variant> >&);

    // misc helpers
    /**
     * create the representation of a thread running in
     * the target process
     */
    RefPtr<ThreadImpl> create_thread(long id, pid_t, int);

    RefPtr<ThreadImpl> handle_fork(pid_t, int status);
    RefPtr<ThreadImpl> handle_exec(pid_t);

    void detach_internal();

protected:
    void wait_for_threads_to_stop(

        ThreadImpl* currentThread,
        const       ThreadImplMap& threadsToStop, 
        bool        queueEvents);

    Runnable::State get_thread_state(ThreadImpl&);

    /**
     * build a collection of threads that have to be stopped
     */
    void collect_threads_to_stop(ThreadImplMap&, Thread* threadToSkip);

    void kill_threads(pid_t, const ThreadList&);

    addr_t setup_caller_frame(Thread&, addr_t sp, long pc);

    void set_ptrace_options(pid_t);

    std::string thread_name(pid_t) const;

    std::vector<std::pair<word_t, word_t> > aux_vect() const;

private:
    virtual pid_t get_signal_sender_pid(const Thread&) const;

    /**
     * handle clone(), fork(), exec() etc
     * @return true if the debugger should continue
     * without stopping and prompting the user
     */
    bool handle_extended_event(Thread&, int eventType);

    bool check_extended_event(Thread*);

    bool is_thread(pid_t) const;
    void on_clone(pid_t);
    void on_fork(pid_t, size_t wordSize, int status);

    void set_status_after_stop(const RefPtr<ThreadImpl>&, int status, bool);

    virtual VirtualDSO* read_virtual_dso() const;

    void cache_main_thread_unique(const td_thrhandle_t&, pid_t);

    std::auto_ptr<DebugRegsBase> get_debug_regs(Thread&) const;

    virtual std::string get_system_release() const;

    bool read_state(pid_t, RunnableState&, std::string& comm) const;

    void update_threads_info();

private:
    bool resumeNewThreads_;
    mutable bool readingAuxVect_;
    mutable Kind kind_;
    mutable std::string commandLine_;
    psaddr_t mainThreadUnique_;

    mutable user_fpxregs_struct fpxregs_; // scratch buffer
    int oldKernel_;
};


#endif // LINUX_LIVE_H__0266AD83_58A4_11DA_B82B_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
