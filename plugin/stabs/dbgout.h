#ifndef DBGOUT_H__482064A4_E0C2_4628_B7B4_CED236F198F0
#define DBGOUT_H__482064A4_E0C2_4628_B7B4_CED236F198F0
//
// $Id: dbgout.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>

#ifndef DBGOUT_H_
#define DBGOUT_H_

#ifdef dbgout
 #undef dbgout
#endif

#ifdef DEBUG
 #define dbgout(level) if (debugLevel_ <= level);\
                       else clog << "Stab::" << __func__ << ": "
#else
// the compiler should optimize it out
 #define dbgout(level) while(0) clog
#endif

#endif// DBGOUT_H_
#endif // DBGOUT_H__482064A4_E0C2_4628_B7B4_CED236F198F0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
