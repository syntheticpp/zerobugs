#ifndef PROCESS_H__FD7E9CF0_D798_4AE2_84A5_32C518278D55
#define PROCESS_H__FD7E9CF0_D798_4AE2_84A5_32C518278D55
//
// $Id: process.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sys/types.h>
#include "zdk/enum.h"
#include "zdk/memio.h"
#include "zdk/shared_string.h"
#include "zdk/zerofwd.h"


/**
 * Describes how the process was started
 */
enum ProcessOrigin
{
    // the process was executed from under the debugger
    ORIGIN_DEBUGGER,

    // the debugger attached to a process that was already
    // running in the system
    ORIGIN_SYSTEM,

    // the process information was read from a core dump
    ORIGIN_CORE,
};


/**
 * Information shared by all the threads that belong to a process.
 *
 * @note that here Process has a meaning that is closer to the Win32
 * paradigm: a process is composed of one or more threads. UNIX has
 * traditionally used "process" and "task" interchangeably. Here,
 * "Process" refers to a group of tasks, out of which all but one
 * have been created with the clone() call. All the tasks are called
 * threads.
 */
DECLARE_ZDK_INTERFACE_(Process, MemoryIO)
{
    DECLARE_UUID("7c21f922-d0b8-456b-bff9-250c4d3ac02a")

    /**
     * @return the process ID
     */
    virtual pid_t pid() const = 0;

    /**
     * @return the image filename
     */
    virtual const char* name() const = 0;

    virtual SharedString* command_line() const = 0;

    virtual ProcessOrigin origin() const = 0;

    virtual SymbolMap* symbols() const = 0;

    /**
     * Get a Thread by its light-weight process ID, or
     * thread ID, whichever the implementation might find
     * more efficient and convenient
     * NULL if none matches.
     */
    virtual Thread* get_thread(pid_t, unsigned long tid = 0) = 0;

    virtual size_t enum_modules(EnumCallback<Module*>*) = 0;

    virtual size_t enum_threads(EnumCallback<Thread*>* = 0) = 0;

    // virtual bool is_multithread(bool reserved = false) const = 0;

    /**
     * get the environment for this process
     */
    virtual const char* const* environment() const = 0;

    virtual BreakPointManager* breakpoint_manager() const = 0;

    virtual bool is_attached(Thread*) const = 0;

    virtual SymbolTable* vdso_symbol_tables() const = 0;

    /**
     * @return the debugger instance attached to this process
     */
    virtual Debugger* debugger() const = 0;

    /**
     * For internal use; Target is opaque.
     */
    virtual Target* target() const = 0;
};

#endif // PROCESS_H__FD7E9CF0_D798_4AE2_84A5_32C518278D55
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
