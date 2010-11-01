//
// $Id: debug_event.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <stdexcept>
#include <boost/python.hpp>
#include "generic/lock.h"
#include "zdk/check_ptr.h"
#include "zdk/get_pointer.h"
#include "debug_event.h"

using namespace boost;
using namespace boost::python;


Mutex DebugEvent::mutex_;
std::vector<int> DebugEvent::pendingCalls_;



DebugEvent::DebugEvent(Type type, Thread* thread, int syscallNum)
    : type_(type)
    , thread_(thread)
    , syscall_(syscallNum)
{
    if (syscallNum >= 0)
    {
        // we get one notification when the Syscall enters, and
        // another one when it completes; we match them with the
        // pendingCalls_ stack

        Lock<Mutex> lock(mutex_);
        if (!pendingCalls_.empty() && (pendingCalls_.back() == syscallNum))
        {
            type_ = SYSCALL_LEAVE;
        }
        else
        {
            pendingCalls_.push_back(syscallNum);
        }
    }
    else if (type == SYSCALL_ENTER)
    {
        // throw std::logic_error("negative syscall number");
        type_ = SYSCALL_LEAVE;
    }

    if (type_ == SYSCALL_LEAVE)
    {
        Lock<Mutex> lock(mutex_);
        if (pendingCalls_.empty())
        {
            throw std::logic_error("empty pending calls stack");
        }
        else
        {
            assert(syscall_ == pendingCalls_.back() || syscall_ < 0);
            syscall_ = pendingCalls_.back();
            pendingCalls_.pop_back();
        }
    }
}


DebugEvent::~DebugEvent() throw()
{
    try
    {
        switch (type_)
        {
        case SYSCALL_ENTER:
            // syscall tracing got turned off?
            if (thread_ &&
                !(thread_->debugger()->options() & Debugger::OPT_TRACE_SYSCALLS)
               )
            {
                Lock<Mutex> lock(mutex_);
                assert(!pendingCalls_.empty());
                pendingCalls_.pop_back();
            }
            break;

        default:
            break;
        }
    }
    catch (...)
    {
    }
}


int DebugEvent::syscall() const
{
    switch (type_)
    {
    case SYSCALL_ENTER:
    case SYSCALL_LEAVE:
        break;

    default:
        PyErr_SetString(PyExc_RuntimeError, "syscall: incorrect event type");
    }
    return syscall_;
}


void export_debug_event()
{
    register_ptr_to_python<RefPtr<DebugEvent> >();

scope in_DebugEvent =
    class_<DebugEvent, bases<>, noncopyable>("DebugEvent")
        .def("process", &DebugEvent::process,
            "return the target process on which the event occurred"
            )
        .def("syscall", &DebugEvent::syscall)
        .def("type", &DebugEvent::type, "return the type of the event")
        .def("thread", &DebugEvent::thread,
            "return the target thread on which the event occurred"
            )
        ;

    enum_<DebugEvent::Type>("Type")
        .value("Update", DebugEvent::UPDATE)
        .value("None", DebugEvent::NONE)
        .value("Signal", DebugEvent::SIGNAL)
        .value("Breakpoint", DebugEvent::BREAKPOINT)
        .value("BreakPoint", DebugEvent::BREAKPOINT)
        .value("Finished", DebugEvent::FINISHED)
        .value("SysCallEnter", DebugEvent::SYSCALL_ENTER)
        .value("SysCallLeave", DebugEvent::SYSCALL_LEAVE)
        .value("SingleStep", DebugEvent::SINGLE_STEP)
        .value("EvalComplete", DebugEvent::EVAL_COMPLETE)
        .value("DoneStepping", DebugEvent::DONE_STEPPING)
        .value("CallReturned", DebugEvent::RETURNED)
        ;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
