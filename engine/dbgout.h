#ifndef DBGOUT_H__B34AAE30_B5DC_472A_B5BD_CC1CA657D232
#define DBGOUT_H__B34AAE30_B5DC_472A_B5BD_CC1CA657D232
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

#include "zdk/eventlog.h"


CLASS DebugChannel : public eventlog::Channel<DebugChannel>
{
    int level_;

public:
    DebugChannel(const char* fn, int level) : level_(level)
    { }

    int level() const { return level_; }

    static void prefix(std::ostream& outs)
    {
        outs << "zero==>  ";
    }
};

#if DEBUG
 #define dbgout(n) \
    eventlog::Stream<DebugChannel>(this->debug_channel(__func__), (n))
#else

 #define dbgout(level) while (0) eventlog::Null<>()
#endif

#endif // DBGOUT_H__B34AAE30_B5DC_472A_B5BD_CC1CA657D232
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
