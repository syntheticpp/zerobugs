#ifndef DARWIN_LIVE_H__57A8E5B0_C454_45E0_B396_4F9991EA0D07
#define DARWIN_LIVE_H__57A8E5B0_C454_45E0_B396_4F9991EA0D07
//
// $Id: darwin_live.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "darwin.h"


class DarwinLiveTarget : public DarwinTarget
{
public:
    virtual ~DarwinLiveTarget() throw();

    virtual word_t result(const Thread&) const;
    virtual void set_result(pid_t, word_t);

    virtual int64_t result64(const Thread&) const;
    virtual void set_result64(pid_t, int64_t);

    virtual long double result_double(const Thread&) const;
    virtual void set_result_double(pid_t, long double);

    virtual ZObject* regs(const Thread&) const;
    virtual ZObject* fpu_regs(const Thread&) const;

    /**
     * Enumerate the general purpose registers, return number of
     * registers. If a non-null pointer to a callback object is given,
     * then call its notify() method for each register.
     */
    virtual size_t enum_user_regs(
        Thread&, EnumCallback<Register*>*) const;

    /**
     * Enumerate the general purpose registers, and the FPU
     * (and possible extended) registers, in the same manner as
     * enum_user_regs
     */
    virtual size_t enum_fpu_regs(
        Thread&, EnumCallback<Register*>*) const;

    /**
     * Modify the n-th general purpose CPU register.
     * Which register is actually modified is platform-specific
     */
    virtual void
        write_register(Thread&, size_t n, Platform::reg_t);
    /**
     * This is a generalized method for modifying the contents
     * of a register in the debugged program, works for general
     * purpose registers as well as for special purpose registers
     * (such as FPU, etc).
     */
    virtual bool write_register(Register&, const Variant&);

    virtual void set_registers(Thread&, ZObject*, ZObject*);

    /**
     * Read the contents of the n-th general purpose register,
     * optionally fetching it from the active's frame saved state.
     */
    virtual Platform::reg_t
        read_register(const Thread&, size_t n, bool) const;

    // methods for retrieving special registers
    virtual addr_t program_count(const Thread&) const;
    virtual addr_t frame_pointer(const Thread&) const;
    virtual addr_t stack_pointer(const Thread&) const;

    /**
     * Force execution to resume at given address,
     * by explicitly modifying the program counter
     */
    virtual void set_program_count(pid_t, addr_t);
    virtual void set_stack_pointer(pid_t, addr_t);

    /**
     * @return true if the dynamic linker/loader generates
     * events and we have successfully installed a breakpoint
     * to intercept them.
     */
    virtual bool has_linker_events() const { return false; }

    /**
     * Attach to running process
     */
    virtual void attach(pid_t) = 0;

    virtual void detach(bool no_throw = false) = 0;

    /**
     * Execute a command (with optional environment) and
     * return the main thread of the newly spawned process
     */
    virtual Thread*
        exec(const ExecArg&, const char* const* env) = 0;

    // todo: remove
    // virtual Thread* get_thread_by_lwpid(pid_t) const = 0;

    virtual Thread* get_thread(pid_t, unsigned long) const = 0;

    /**
     * thread ID to light-weight process ID
     */
    virtual pid_t tid_to_lwpid(long) const = 0;

    /**
     * map PID to Thread in which event has occurred
     * (may internally require a PID to LWPID translation).
     */
    virtual Thread* event_pid_to_thread(pid_t) const = 0;

    /**
     * Enumerate all the attached threads, return thread count.
     */
    virtual size_t enum_threads(EnumCallback<Thread*>* = NULL) = 0;

    virtual bool is_attached(Thread* = NULL) const = 0;

    // todo: needed?
    // virtual void check_for_new_threads() = 0;

    /**
     * Stop all threads in the target, optionally
     * passing in the thread that received an event.
     */
    virtual bool stop_all_threads(Thread* = NULL) = 0;

    /**
     * Tell all threads to stop, but don't wait.
     */
    virtual void stop_async() = 0;

    /**
     * @return number of actual resumed threads.
     */
    virtual size_t resume_all_threads() = 0;

    /**
     * Called when a thread finishes execution.
     * @return the remaining number of running threads in this target
     */
    virtual size_t cleanup(Thread&) = 0;

    /**
     * @return true when the handling of an event requires all
     * threads in the target to be stopped by the debugger
     * @note the purpose of the method is to optimize handling
     * of events such as creation of new threads.
     */
    virtual bool event_requires_stop(Thread*) = 0;

    virtual void handle_event(Thread*) = 0;

    enum segment_t { DATA_SEGMENT, CODE_SEGMENT };

    virtual void read_memory(
        pid_t       lwpid,
        segment_t   seg,
        addr_t      address,
        word_t*     buffer,
        size_t      howManyWords,
        size_t*     wordsRead = 0) const = 0;

    virtual void write_memory(
        pid_t       lwpid,
        segment_t   seg,
        addr_t      addr,
        const word_t* buf,
        size_t      wordsToWrite) = 0;
};

#endif // DARWIN_LIVE_H__57A8E5B0_C454_45E0_B396_4F9991EA0D07
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
