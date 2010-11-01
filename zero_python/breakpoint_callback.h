#ifndef BREAKPOINT_CALLBACK_H__72ED0D1D_EBC7_42AC_BECB_0B6545DF6D5B
#define BREAKPOINT_CALLBACK_H__72ED0D1D_EBC7_42AC_BECB_0B6545DF6D5B
//
// $Id: breakpoint_callback.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/mutex.h"
#include "zdk/weak_ptr.h"
#include "handle_error.h"
#include "marshaller.h"
#include "utility.h"

using namespace boost::python;


void set_temp_breakpoint_(RefPtr<Thread>);


/**
 * BreakPointAction that invokes a callable Python object.
 * Used by set_breakpoint_ (first overload, see thread.cpp, process.cpp)
 */
class BreakPointCallback : public ZObjectImpl<BreakPointAction>
{
    object obj_;

    bool run(RefPtr<Thread> thread, RefPtr<BreakPoint> bpnt)
    {
        assert(thread);

        try
        {
            if (call<bool>(obj_.ptr(), thread, bpnt))
            {
                // if the user-defined callback returns True,
                // then set a temporary breakpoint at the
                // current program counter, to go interactive

                ThreadMarshaller::instance().send_command(
                    boost::bind(set_temp_breakpoint_, thread), __func__);
            }
        }
        catch (const error_already_set&)
        {
            python_handle_error();
        }
        return true;
    }

public:
    explicit BreakPointCallback(PyObject* obj)
        : obj_(handle<>(borrowed(obj)))
    { }

    ~BreakPointCallback() throw() { }

    const char* name() const
    {
        return to_string(obj_);
    }

    word_t cookie() const { return (word_t)obj_.ptr(); }

    bool execute(Thread* thread, BreakPoint* bpnt)
    {
    #ifdef DEBUG
        std::clog << "--- CALLBACK ENTER ---\n";
    #endif
        if (thread)
        {
            ThreadMarshaller::instance().send_event(
                boost::bind(&BreakPointCallback::run, this, thread, bpnt),
                "execute-breakpoint");
        }
    #ifdef DEBUG
        std::clog << "--- CALLBACK LEAVE ---\n";
    #endif
        return true;
    }
};


/**
 * Helper callback for enumerating thread breakpoints and
 * collecting them into a Python list
 */
class BreakPointCollector : public EnumCallback<volatile BreakPoint*>
{
    mutable Mutex mutex_;
    boost::python::list list_;

public:
    void notify(volatile BreakPoint* breakpoint)
    {
        Lock<Mutex> lock(mutex_);
        list_.append(WeakPtr<BreakPoint>(const_cast<BreakPoint*>(breakpoint)));
    }

    boost::python::list list() const
    {
        Lock<Mutex> lock(mutex_);
        return list_;
    }
};
#endif // BREAKPOINT_CALLBACK_H__72ED0D1D_EBC7_42AC_BECB_0B6545DF6D5B

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
