#ifndef RUNNABLE_H__27C6EF27_3E43_4A44_A4B9_D491365542C3
#define RUNNABLE_H__27C6EF27_3E43_4A44_A4B9_D491365542C3
//
// $Id: runnable.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/enum.h"
#include "zdk/platform.h"

using Platform::addr_t;
using Platform::reg_t;
using Platform::word_t;

struct Process;
struct Register;
struct Thread;
struct Variant;


/**
 * A runnable task (lightweight process)
 */
DECLARE_ZDK_INTERFACE_(Runnable, Unknown2)
{
    DECLARE_UUID("18c71c95-2880-482b-bb15-764480c16510")

    /**
     * @note values are unix specific, this
     * enum might go away in future versions
     */
    enum State
    {
        UNKNOWN,
        RUNNING = 'R',
        SLEEPING_INTERRUPTIBLE = 'S',
        SLEEPING_UNINTERRUPTIBLE = 'D',
        ZOMBIE = 'Z',
        TRACED_OR_STOPPED = 'T'
    };

    virtual pid_t pid() const = 0;
    virtual pid_t gid() const = 0;
    virtual pid_t ppid() const = 0;

    virtual const char* name() const = 0;

    virtual State runstate() const = 0;

    /**
     * @return the associated process
     */
    virtual Process* process() const = 0;

    /**
     * @return the associated thread
     */
    virtual Thread* thread() const = 0;

    /**
     * @return the size of the virtual memory
     * owned by this process.
     */
    virtual size_t vmem_size() const = 0;

    /**
     * The number of clock ticks that this process
     * has been scheduled in user mode so far.
     * @see man clock(3)
     */
    virtual size_t usr_ticks() const = 0;

    /**
     * The number of clock ticks that this process
     * has been scheduled in system mode.
     * @see man clock(3)
     */
    virtual size_t sys_ticks() const = 0;

    virtual addr_t stack_start() const = 0;

    /**
     * Enumerate the files currently open by this task; for each
     * file, call the provided callback object (if not NULL) with
     * the file descriptor and filename. The filename may be NULL
     * for pipes, etc.
     * @return the number of files
     */
    virtual size_t
        enum_open_files(EnumCallback2<int, const char*>*) const = 0;

    /**
     * This method abstracts out modifying the register that is
     * used by convention for return values (e.g. on x86 architectures,
     * that would be EAX; on x86-64, RAX).
     */
    virtual void set_result(word_t) = 0;

    /**
     * Abstract out modifying the registers that store a 64-bit
     * function return value (e.g. on x86, that means modifying
     * the EDX and EAX registers, on x86-64 it is equivalent to
     * set_result().
     */
    virtual void set_result64(int64_t) = 0;

    /**
     * Modify the FPU register(s) that contain a floating point
     * result (%ST(0) on the x86, for e.q).
     *
     * Depending upon the architecture, doubles and long-doubles
     * may be represented / stored by different registers
     */
    virtual void set_result_double(long double, size_t) = 0;

    /**
     * Force execution to resume at given address,
     * by explicitly modifying the program counter
     * register (EIP on the Intel x86 processors.)
     */
    virtual void set_program_count(addr_t) = 0;

    /**
     * Force the value of the stack pointer register to
     * the specified memory address
     */
    virtual void set_stack_pointer(addr_t) = 0;

    /**
     * modify the value stored inside specified CPU register
     * @return false if register is read-only
     */
    virtual bool write_register(Register*, const Variant*) = 0;

    /**
     * modify value in N-th cpu register
     * @note it only works for general-purpose registers;
     * this method cannot be used to modify FPU regs.
     */
    virtual void write_register(size_t n, reg_t) = 0;

    /**
     * @todo document
     */
    virtual void set_registers(ZObject*, ZObject*) = 0;

    /**
     * Set the single stepping mode of this thread,
     * optionally specifying an action context. The
     * action context may be used later to identify
     * the entity that initiated the single-stepping.
     */
    virtual void set_single_step_mode(bool, ZObject* = NULL) = 0;

    /**
     * Execute one single machine instruction
     */
    virtual void step_instruction() = 0;

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
    virtual void step_until_current_func_returns() = 0;

    /**
     * @note not implemented
     */
    virtual void freeze(bool) = 0;

    /**
     * @note not implemented
     */
    virtual bool is_frozen() const = 0;

    virtual void clear_cached_regs() = 0;

    virtual bool resume() = 0;

    /**
     * Set next line destination when stepping over a line
     * of source code (without stepping into function calls).
     */
    virtual void set_next(Symbol*) = 0;

    virtual void set_stepping(bool) = 0;
};


#endif // RUNNABLE_H__27C6EF27_3E43_4A44_A4B9_D491365542C3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
