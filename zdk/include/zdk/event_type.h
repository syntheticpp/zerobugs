#ifndef EVENT_TYPE_H__5B65850C_28E0_4E02_A1CA_7F8761F7A0D9
#define EVENT_TYPE_H__5B65850C_28E0_4E02_A1CA_7F8761F7A0D9
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <limits.h>


enum EventType
{
    // no event, just prompting the user for a command
    E_PROMPT,

    // a debugged thread stopped because it has received a signal
    E_THREAD_STOPPED,

    // the thread stopped in the debugger with a
    //   SIGTRAP as the result of hitting a breakpoint
    E_THREAD_BREAK,

    // the thread finished running
    E_THREAD_FINISHED,

    E_SYSCALL,

    E_SINGLE_STEP,

    // used internally by the expression evaluator
    //   to indicate that a function call or some other
    //   asynchronous evaluation has completed.
    E_EVAL_COMPLETE,

    // returned from function call
    E_THREAD_RETURN,

    // thread is about to exit
    E_THREAD_EXITING,

    // there are no more threads left in the target
    E_TARGET_FINISHED,

    // force to sizeof(uint)
    E_NONE = UINT_MAX,
    E_DEFAULT = E_NONE
};

#endif // EVENT_TYPE_H__5B65850C_28E0_4E02_A1CA_7F8761F7A0D9

