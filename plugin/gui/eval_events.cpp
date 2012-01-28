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
#include <iostream>
#include <sstream>
#include <signal.h>
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "zdk/variant.h"
#include "zdk/zero.h"
#include "eval_events.h"

using namespace std;


EvalEvents::~EvalEvents() throw()
{
}


bool EvalEvents::on_done(Variant* var, bool*, DebugSymbolEvents*)
{
    Lock<Mutex> lock(mutex_);
    bool result = active_;

    if (var && active_)
    {
        sig_done.emit(*var);
        active_ = false;
    }
    return result;
}


void EvalEvents::on_error(const char* msg)
{
    Lock<Mutex> lock(mutex_);
    if (active_ && msg)
    {
        if (!sig_error.emit(msg))
        {
            active_ = false;
        }
    }
}


void EvalEvents::on_warning(const char* msg)
{
    Lock<Mutex> lock(mutex_);
    if (active_ && msg)
    {
        sig_warning.emit(msg);
    }
}


void EvalEvents::on_call(addr_t addr, Symbol* symbol)
{
    Lock<Mutex> lock(mutex_);
    addr_ = addr;
}


bool EvalEvents::on_event(Thread* thread, addr_t addr)
{
    Lock<Mutex> lock(mutex_);
    if (!active_)
    {
        return false;
    }
    const int signum = CHKPTR(thread)->signal();

    if (thread != thread_.get())
    {
 #ifdef DEBUG
        clog << "signal " << signum;
        clog << " in thread=" << thread->lwpid() << endl;
 #endif
    }
    else if ((addr != addr_) && signum)
    {
        ostringstream msg;

        if (RefPtr<SharedString> ss = thread_get_event_description(*thread))
        {
             msg << ss->c_str();
        }
        else
        {
            msg << "Expression evaluation interrupted by signal ";
            msg << thread->signal() << " at " << hex << addr;
        }
        msg << " in thread " << dec << thread->lwpid() << ". ";

        if (thread_finished(*thread))
        {
            msg << "Thread has finished.";
        }
        else
        {
            if (thread->signal() == SIGTRAP)
            {
                bool cont = true;
                string m = msg.str() + "\nContinue?";
                sig_confirm(m, &cont, NULL);

                if (!cont)
                {
                    active_ = false;
                    sig_deactivate.emit();
                }
                return cont;
            }
            // msg << "CPU context will now be restored.";

            // attempt to restore state -- this is not foolproof,
            // some side-effect may have already happened but it's
            // the best that we can do; the other registers are handled
            // by the OnCallReturn action in the interpreter
            thread->set_signal(0);
            interface_cast<Runnable&>(*thread).set_program_count(addr_);
        }
        sig_error.emit(msg.str());
    }
    return true;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
