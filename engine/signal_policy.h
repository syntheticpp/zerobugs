#ifndef SIGNAL_POLICY_H__C78CCA9A_B6CE_403A_B953_83E2FF4AAD66
#define SIGNAL_POLICY_H__C78CCA9A_B6CE_403A_B953_83E2FF4AAD66
//
// $Id: signal_policy.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iosfwd>
#include <boost/utility.hpp>    // boost::noncopyable
#include "zdk/signal_policy.h"

struct Debugger;


/**
 * A class that describes the policy for handling a signal
 * (received by the debugged program). The debugger can
 * choose to stop or not, and to re-deliver the signal
 * back to debuggee or not.
 */
CLASS SignalPolicyImpl : public SignalPolicy, boost::noncopyable
{
public:
    SignalPolicyImpl(Debugger&, int);

    virtual ~SignalPolicyImpl() throw () {}

    bool pass() const { return pass_; }

    void set_pass(bool flag);

    bool stop() const { return stop_; }

    void set_stop(bool flag);

    bool can_ignore() const throw();

private:
    Debugger&   dbg_;
    int         sigNum_;
    bool        pass_; // pass signal to debugged prog?
    bool        stop_; // stop in debugger on signal?
};


std::ostream& operator<<(std::ostream&, const SignalPolicy&);

ZDK_LOCAL bool is_pthread_signal(int sig);

#endif // SIGNAL_POLICY_H__C78CCA9A_B6CE_403A_B953_83E2FF4AAD66
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
