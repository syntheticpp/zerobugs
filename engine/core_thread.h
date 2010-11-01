#ifndef CORE_THREAD_H__B9B4E072_1D2B_4133_9B3F_91FA9410AC27
#define CORE_THREAD_H__B9B4E072_1D2B_4133_9B3F_91FA9410AC27
//
// $Id: core_thread.h 714 2010-10-17 10:03:52Z root $
//
// Support for reading thread information from corefiles
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/shared_ptr.hpp>
#include "zdk/config.h"
#include "target/corefwd.h"
#include "process.h"
#include "thread_base.h"

class DebuggerBase;
class StackTraceImpl;


/**
 * The ghost of thread that crashed, read from a core dump.
 */
CLASS CoreThreadImpl : public ThreadBase
{
public:
    DECLARE_UUID("a2f996e8-4be4-4c19-b121-4d7dd14af79c")

    BEGIN_INTERFACE_MAP(CoreThreadImpl)
        INTERFACE_ENTRY(CoreThreadImpl)
        INTERFACE_ENTRY(Thread)
        INTERFACE_ENTRY_DELEGATE(process())
    END_INTERFACE_MAP()

    CoreThreadImpl( Target&,
                    CorePtr,
                    size_t, // index of this thread
                    const prstatus_t&,
                    SymbolMap* symbols = 0);

    ~CoreThreadImpl() throw();

    /**
     * @return the lightweight process ID
     */
    virtual pid_t lwpid() const;

    virtual pid_t ppid() const;
    virtual pid_t gid() const;

    virtual unsigned long thread_id() const;

    virtual Runnable::State runstate() const;

    virtual void detach();

    /**
     * @return the thread's name (the same
     * as the executable's filename).
     */
    virtual const char* filename() const;

    virtual bool is_live() const { return false; }

    /**
     * @return false always -- assuming that for forked
     * threads that crash, the kernel dumps separate core files.
     */
    virtual bool is_forked() const { return false; }

    /**
     * @return false always -- assuming that for execved
     * threads that crash, the kernel dumps separate core files.
     */
    virtual bool is_execed() const { return false; }

    /**
     * Get the return address of the current function
     * @note meaningful only in the context of a function
     */
    virtual addr_t ret_addr();

    virtual addr_t stack_start() const;

    /**
     * @return value stored in cpu register N
     */
    virtual reg_t read_register(int n, bool) const;

    /**
     * Returns the status, last obtained by waitpid()
     * when the thread stopped or exited.
     */
    virtual int status() const;

    /**
     * @return last signal received by this thread
     */
    virtual int signal() const;

    virtual void set_signal(int32_t);

    virtual bool single_step_mode() const;

    virtual bool is_line_stepping() const
    { return false; }

    /**
     * Returns true if an event on this thread has
     * been queued (because there where several events
     * that occurred at about the same time on
     * different threads).
     */
    virtual bool is_event_pending() const;

    virtual bool is_stopped_by_debugger() const { return false; }

    /**
     * Get a stack trace for this thread, containing
     * the specified number of frames.
     */
    virtual StackTrace* stack_trace(size_t = UINT_MAX) const;

    virtual size_t stack_trace_depth() const;

    virtual DebugSymbol* func_return_value();

    /**
     * Associate a value with this thread.
     * @param key a user-supplied key name
     * @param val a user-supplied value.
     * @param replace if another value of the same
     * key is found, replace it if this param is true
     */
    virtual void set_user_data(
        const char* key,
        word_t      val,
        bool        replace = true);

    /**
     * Retrieve user data by key.
     * @return true if found, false if no such key.
     */
    virtual bool get_user_data(const char* key, word_t*) const;

    /**
     * Associate a reference-counted object with this thread.
     */
    virtual void set_user_object(
        const char* keyName,
        ZObject*    object,
        bool        replace = true);

    /**
     * Get user object by key.
     * @return pointer to ref-counted object if found, or
     *  NULL if no such key.
     */
    virtual ZObject* get_user_object(const char*) const;

    virtual size_t enum_cpu_regs(EnumCallback<Register*>*);

    virtual Target* target() const { return target_.ref_ptr().get(); }

    virtual ZObject* action_context() const { return NULL; }

    virtual bool is_done_stepping() const { return false; }

    virtual bool is_exiting() const { return false; }

    virtual bool exited(int* status = NULL) const;

private:
    CorePtr         core_;  // fixme: duplicated in target
    WeakPtr<Target> target_;
    const size_t    index_; // index of this thread

    const pid_t     lwpid_;
    pid_t           ppid_;
    gid_t           gid_;
    const short int cursig_; // current signal

    mutable RefPtr<StackTraceImpl>  trace_;
};

#endif // CORE_THREAD_H__B9B4E072_1D2B_4133_9B3F_91FA9410AC27
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
