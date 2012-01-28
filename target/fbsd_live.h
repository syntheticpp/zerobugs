#ifndef FBSD_LIVE_H__D2077863_5AE6_11DA_8F2D_00C04F09BBCC
#define FBSD_LIVE_H__D2077863_5AE6_11DA_8F2D_00C04F09BBCC
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
#define TD_EVENTS_ENABLE (td_thr_events_e)31

#include "fbsd.h"
#include "linker_events.h"
#include "thread_events.h"


/**
 * Bridge between LinuxTarget objects and thread_db library.
 */
struct ps_prochandle : public FreeBSDTarget
{
    RefPtr<SymbolTable> symtab_;

protected:
    ps_prochandle(debugger_type& dbg) : FreeBSDTarget(dbg) { }
};

typedef ThreadEventObserver<ps_prochandle> FreeBSDLiveBase;


/**
 * Implement FreeBSD live target
 */
class FreeBSDLiveTarget : public LinkerEventObserver<FreeBSDLiveBase>
{
    typedef LinkerEventObserver<FreeBSDLiveBase> Super;

    DECLARE_VISITABLE()

public:
    explicit FreeBSDLiveTarget(debugger_type&, unsigned wordSize);

    ~FreeBSDLiveTarget() throw() { }

    virtual word_t get_breakpoint_opcode(word_t) const;

    virtual void detach(bool no_throw = false);

    /**
     * Translate process ID to the light-weight process
     * that got an event.
     */
    virtual pid_t pid_to_lwpid(pid_t) const;

    virtual RefPtr<SymbolMap> read_symbols();
    virtual void read_environment(SArray&) const;

    RefPtr<ProcessImpl> new_process(pid_t, const ExecArg*, ProcessOrigin);

    RefPtr<Thread> new_thread(long id, pid_t, int status);
    virtual bool read_state(const Thread&, RunnableState&) const;
    virtual pid_t get_signal_sender_pid(const Thread&) const;

private:
    //virtual void step_until_safe(Thread&, addr_t) const;

    virtual void read_memory(
        pid_t       lwpid,
        SegmentType segType,
        addr_t      address,
        word_t*     buffer,
        size_t      howManyWords,
        size_t*     wordsRead = 0) const;

    virtual void write_memory(
        pid_t       lwpid,
        SegmentType segType,
        addr_t      addr,
        const word_t* buf,
        size_t      wordsToWrite);

    /**
     * modify cpu register's content
     */
    virtual bool write_register(Register&, const Variant&);
    virtual void write_register(Thread&, size_t, reg_t);
    virtual bool read_register( const Thread&,
                                int regnum,
                                bool fromStackFrame,
                                reg_t&) const;

    virtual ZObject* regs(const Thread&) const;
    virtual ZObject* fpu_regs(const Thread&) const;

    /**
     * Execute a command (with optional environment) and
     * return the main thread of the newly spawned process
     */
    virtual Thread* exec(const ExecArg&, const char* const*);

    virtual void attach(pid_t);

    /**
     * Stop all threads in the target, optionally
     * passing in the thread that received an event.
     */
    virtual bool stop_all_threads(Thread*);
    /**
     * Tell all threads to stop, but don't wait.
     */
    virtual void stop_async();

    virtual size_t resume_all_threads();

    virtual size_t cleanup(Thread&);

    virtual bool event_requires_stop(Thread*);

    /**
     * map PID to Thread in which event has occurred
     * (may internally require a PID to LWPID translation).
     */
    virtual Thread* event_pid_to_thread(pid_t) const;

    virtual addr_t setup_caller_frame(Thread&, addr_t sp, long pc);
    virtual std::auto_ptr<DebugRegsBase> get_debug_regs(Thread&) const;

    /* get the IDs of light-weight processes */
    size_t get_lwp_list(std::vector<pid_t>&);

    bool map_lwpid(pid_t pid, td_thrhandle_t& th) const;
    unsigned long lwpid_to_tid(lwpid_t pid) const;

    bool map_tid(long thread_id, td_thrhandle_t& th) const;
    pid_t tid_to_lwpid(long id) const;

private:
    RefPtr<ThreadImpl> create_thread(long id, pid_t, int);

    bool resumeNewThreads_;
};


#endif // FBSD_LIVE_H__D2077863_5AE6_11DA_8F2D_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
