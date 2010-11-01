#ifndef DEBUG_EVENT_H__E6BC3229_3F6A_4785_8DA7_51DC663B3B92
#define DEBUG_EVENT_H__E6BC3229_3F6A_4785_8DA7_51DC663B3B92
//
// $Id: debug_event.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <vector>
#include "zdk/mutex.h"
#include "zdk/zero.h"
#include "zdk/zobject_impl.h"


void export_debug_event();


class ZDK_LOCAL DebugEvent : public ZObjectImpl<>
{
public:
    enum Type
    {
        UPDATE = E_PROMPT,
        NONE = E_NONE,
        SIGNAL = E_THREAD_STOPPED,
        BREAKPOINT = E_THREAD_BREAK,
        FINISHED = E_THREAD_FINISHED,
        SYSCALL_ENTER = E_SYSCALL,
        SINGLE_STEP = E_SINGLE_STEP,
        EVAL_COMPLETE = E_EVAL_COMPLETE,
        RETURNED = E_THREAD_RETURN,

        // these do not map to any ZDK-defined event types:
        DONE_STEPPING,
        SYSCALL_LEAVE
    };

    DebugEvent() : type_(NONE), syscall_(-1)
    { }

    DebugEvent(Type, Thread*, int = -1);

    virtual ~DebugEvent() throw();

    Type type() const { return type_; }

    RefPtr<Thread> thread() const { return thread_; }

    RefPtr<Process> process() const
    {
        RefPtr<Process> proc;
        if (thread_)
        {
            proc = thread_->process();
        }
        return proc;
    }

    int syscall() const;

private:
    Type type_;
    RefPtr<Thread> thread_;
    int syscall_; // meaningful only for SYSCALL_ENTER/LEAVE

    static Mutex mutex_;
    static std::vector<int> pendingCalls_;
};

#endif // DEBUG_EVENT_H__E6BC3229_3F6A_4785_8DA7_51DC663B3B92
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
