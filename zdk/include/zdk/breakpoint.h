#ifndef BREAKPOINT_H__52AA2A18_0D3B_43C4_847D_C9B9430AC537
#define BREAKPOINT_H__52AA2A18_0D3B_43C4_847D_C9B9430AC537

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/enum.h"
#include "zdk/platform.h"
#include "zdk/watchtype.h"
#include "zdk/zobject.h"
#include <limits.h> // UINT_MAX

#ifdef ANY
 #undef ANY
#endif

struct BreakPoint;
struct Runnable;
struct Symbol;
struct Thread;

using Platform::addr_t;
using Platform::word_t;

/**
 * Optional interface that a breakpoint or watchpoint
 * action may expose -- use query_interface or
 * interface_cast<> to detect it.
 */
DECLARE_ZDK_INTERFACE_(BreakPointActionInfo, Unknown2)
{
    DECLARE_UUID("74a8de61-a8c1-4738-9e49-b2627067f13b")

    virtual const char* description() const = 0;

    virtual bool is_global() const = 0;
};


/**
 * A BreakPointAction encapsulates an action to be taken by
 * the debugger when a breakpoint is hit. A breakpoint may
 * have an arbitrary list of actions.
 */
DECLARE_ZDK_INTERFACE_(BreakPointAction, ZObject)
{
    DECLARE_UUID("b5717f44-4218-461c-9577-2c27f87534bd")

	typedef BreakPointActionInfo Info;

    virtual const char* name() const = 0;

    /**
     * Executes the action when the breakpoint is hit.
     * If the method returns false, the action is removed
     * from the breakpoint list of actions (and it is not
     * executed again when the breakpoint is hit). If all
     * actions are removed, the breakpoint itself is erased.
     */
    virtual bool execute(Thread*, BreakPoint*) = 0;

    /**
     * A "cookie" is an arbitrary number attached to a
     * breakpoint action.
     * @note currently, the only use for cookies is
     * to ensure action uniqueness within a breakpoint.
     */
    virtual word_t cookie() const = 0;
};


/**
 * Models software and hardware breakpoints / watchpoints.
 */
DECLARE_ZDK_INTERFACE_(BreakPoint, ZObject)
{
    DECLARE_UUID("c092c7ae-30b5-4c1b-a7b4-1de65f2d6c44")

    enum Type
    {
        SOFTWARE    = 0x0001,
        EMULATED    = 0x0002,
        HARDWARE    = 0x0004,
        GLOBAL      = SOFTWARE,
        PER_THREAD  = EMULATED,
        ANY         = UINT_MAX, // force enumerated type's size
    };

    typedef ::BreakPointAction Action;

    /**
     * Returns the thread where this breakpoint was set.
     * @note it may not necesarilly mean the thread to
     * which the breakpoint applies. For global (SOFTWARE)
     * breakpoints, all it means it the thread that was
     * current when the breakpoint was set.
     */
    virtual Thread* thread() const volatile = 0;

    virtual addr_t addr() const volatile = 0;

    virtual Type type() const volatile = 0;

    virtual Symbol* symbol() const volatile = 0;

    virtual bool is_enabled() const volatile = 0;

    virtual bool is_deferred() const volatile = 0;

    /**
     * Enables a breakpoint, returns enable count.
     * @note a value of less than or equal to zero
     * means that the breakpoint is disabled.
     */
    virtual int enable() volatile = 0;

    /**
     * Decrements internal count, and returns new value.
     * If counter reaches zero, the breakpoint is disabled.
     * The actual actions taken to disable the breakpoint
     * depends on the type (software or hardware).
     */
    virtual int disable() volatile = 0;

    /**
     * Enumerates the actions associated with this breakpoint,
     * that match the given name (NULL matches all)
     * and calls the callback interface (if not NULL) for each
     * action; returns the number of actions.
     */
    virtual size_t enum_actions(
        const char* name = 0,
        EnumCallback<Action*>* = 0) const volatile = 0;

    virtual size_t action_count() const volatile = 0;

    /**
     * Retrieve action by index
     */
    virtual const Action* action(size_t idx) const volatile = 0;

    /**
     * @return true if action added (may be rejectect if
     * cookie is duplicate)
     */
    virtual bool add_action(Action*) = 0;

    /**
     * Removes all actions with given name.
     * Returns count of removed actions.
     */
    virtual size_t remove_actions(const char* name) volatile = 0;

    virtual size_t remove(Action*, word_t = 0) volatile = 0;

    virtual void execute_actions(Thread*) = 0;

    /**
     * An action context is an arbitrary, user-provided
     * object, associated with a given action. Such an object may
     * be used to track the "context" for a breakpoint action
     * (including for example, the plugin that set the action, the
     * reason for executing the action, etc).
     */
    virtual ZObject* action_context(Action*) const = 0;
};



/**
 * The debugger engine aggregates a breakpoint manager.
 * The interface provides methods for setting and removing
 * breakpoints.
 */
DECLARE_ZDK_INTERFACE_(BreakPointManager, ZObject)
{
    DECLARE_UUID("082eb891-3142-4aa6-8a4e-981efeb95ed4")

    /**
     * Inserts action point at specified address.
     * @note The type is merely a hint. Resources for setting
     * hardware breakpoints are limited, the debugger may
     * choose to emulate a hardware breakpoint in software.
     * @note software breakpoints are global -- i.e. they
     * apply to all threads in the program, whereas hard
     * (or emulated) breakpoints only aplly to the given thread.
     */
    virtual BreakPoint* set_breakpoint(
        Runnable*,
        BreakPoint::Type,
        addr_t,
        BreakPoint::Action*,
        bool deferred = false,
        Symbol* = NULL /* supersedes addr, if not null */) = 0;

    /**
     * Set a memory breakpoint. The execution breaks in the
     * debugger when the specified memory address is being
     * read or written. Actions can be specified, to inspect
     * the memory and check for a specific value.
     * @note This method needs a hardware
     * breakpoint register to be available, or it will fail
     * (and return NULL).
     * @param thread
     * @param addr memory address
     * @param action optional, user-defined action that gets
     * executed when the watchpoint is hit.
     */
    virtual BreakPoint* set_watchpoint(
        Runnable*   thread,
        WatchType   type,
        bool        global,
        addr_t      addr,
        BreakPoint::Action* action = 0) = 0;

    /**
     * This method enumerates breakpoints by thread,
     * address and type. If thread is null, any global or
     * per-thread breakpoint matches; if address is
     * not zero, only breakpoints that match the
     * address are enumerated; if type is ANY all match.
     * @return the number of matching breakpoints.
     * @note this method does not enumerate memory
     * breakpoints (a.k.a. watchpoints)
     */
    virtual size_t enum_breakpoints(
        EnumCallback<volatile BreakPoint*>* = 0,
        Thread* = 0,
        addr_t = 0,
        BreakPoint::Type = BreakPoint::ANY) const = 0;

    /**
     * Enumerate all watchpoints, calls back the notify()
     * method of the EnumCallback object.
     * @return the number of watch points.
     */
    virtual size_t enum_watchpoints(
        EnumCallback<volatile BreakPoint*>* = 0) const = 0;

    /**
     * @note after removing the actions, the breakpoints that
     * have no actions associated are removed
     */
    virtual size_t remove_breakpoint_actions(
        pid_t processID,    // 0 matches all processes
        pid_t lwpThreadID,  // 0 matches all threads
        addr_t addr,        // 0 matches all addresses
        const char* name    // NULL matches all actions
    ) = 0;
};

#endif // BREAKPOINT_H__52AA2A18_0D3B_43C4_847D_C9B9430AC537

