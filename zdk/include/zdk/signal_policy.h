#ifndef SIGNAL_POLICY_H__228F9430_846A_4AD7_8D4A_43BC41DE1022
#define SIGNAL_POLICY_H__228F9430_846A_4AD7_8D4A_43BC41DE1022
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

#include "zdk/unknown2.h"

/**
 * Interface for controlling policies for handling
 * signals received by the debugged program.
 */
DECLARE_ZDK_INTERFACE_(SignalPolicy, struct Unknown)
{
public:
    DECLARE_UUID("0193229f-8417-40cb-84f8-a2de41b4a27f")

    /// @return true if the signal is to be passed
    /// to the debugged program; if false, the signal
    /// is "absorbed" by the debugger and not delivered
    /// to the debugged thread.
    virtual bool pass() const  = 0;
    virtual void set_pass(bool) = 0;

    /// @return true if the debugger should break the
    /// execution upon receipt of the signal.
    virtual bool stop() const  = 0;
    virtual void set_stop(bool) = 0;

    /// Some signals cannot be ignored (i.e. not passed
    /// to the debuggee) without disturbing normal functioning.
    /// For example, at least one version of the pthread
    /// library uses some signals internally for thread
    /// synchronization. This method returns false if the
    /// correspoding signal is one of the known "critical"
    /// ones that should always be passed through.
    virtual bool can_ignore() const throw () = 0;
};

#endif // SIGNAL_POLICY_H__228F9430_846A_4AD7_8D4A_43BC41DE1022
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
