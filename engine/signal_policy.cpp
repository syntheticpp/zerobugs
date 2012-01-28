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
#include <signal.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "dharma/sigutil.h"
#include "zdk/zero.h"
#include "zdk/properties.h"
#include "signal_policy.h"


using namespace std;

#ifndef SIGRTMIN
 #define SIGRTMIN   SIGTHR
#endif

int __pthread_sig_restart = SIGRTMIN - 3;
int __pthread_sig_cancel  = SIGRTMIN - 2;
int __pthread_sig_debug   = SIGRTMIN - 1;

static const char STOP[] = "s";
static const char PASS[] = "p";


bool is_pthread_signal(int sig)
{
    return (sig == __pthread_sig_cancel)
        || (sig == __pthread_sig_restart)
        || (sig == __pthread_sig_debug);
}



ostream& operator<<(ostream& outs, const SignalPolicy& pol)
{
    if (!pol.stop())
    {
        outs << "no";
    }
    outs << "stop\t";

    if (!pol.pass())
    {
        outs << "no";
    }
    outs << "pass";
    return outs;
}



static string policy_name(const char* type, int num)
{
    ostringstream str;
    str << "sig" << num << "." << type;

    return str.str();
}



SignalPolicyImpl::SignalPolicyImpl(Debugger& dbg, int sigNum)
    : dbg_(dbg)
    , sigNum_(sigNum)
 // , pass_(true)
 // , stop_(true)
    , pass_(dbg.properties()->get_word(policy_name(PASS, sigNum).c_str(), 1))
    , stop_(dbg.properties()->get_word(policy_name(STOP, sigNum).c_str(), 1))
{
    if (is_pthread_signal(sigNum_))
    {
        stop_ = false;
    }
    else if (sigNum == SIGCHLD || sigNum == SIGPROF)
    {
        stop_ = false;
    }
}



void SignalPolicyImpl::set_pass(bool flag)
{
    if (!flag)
    {
        if (!can_ignore())
        {
            ostringstream msg;
            msg << sig_description(sigNum_) << ": cannot ignore";

            throw logic_error(msg.str());
        }
    }
    pass_ = flag;
    dbg_.properties()->set_word(policy_name(PASS, sigNum_).c_str(), flag);
}



void SignalPolicyImpl::set_stop (bool flag)
{
    if (!flag)
    {
         if (sigNum_ == SIGTRAP || sigNum_ == SIGSTOP)
         {
            throw logic_error("Always stop at this signal");
         }
    }
    stop_ = flag;
    dbg_.properties()->set_word(policy_name(STOP, sigNum_).c_str(), flag);
}



bool SignalPolicyImpl::can_ignore() const throw()
{
    if (is_pthread_signal(sigNum_)
        || sigNum_ == SIGTRAP || sigNum_ == SIGSTOP)
    {
        return false;
    }

    return true;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
