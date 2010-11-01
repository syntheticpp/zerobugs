#ifndef STATES_H__FBE73FC3_8BEF_4C0C_A6AB_DDB0FCF96918
#define STATES_H__FBE73FC3_8BEF_4C0C_A6AB_DDB0FCF96918
//
// $Id: states.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

enum UIStateMask
{
    uisNone          = 0x0000,
    uisAttached      = 0x0001,
    uisAttachedLive  = 0x0009,
    uisWatchPoints   = 0x0002,
    uisBreakPoints   = 0x0004,
    uisThreadLive    = 0x0008,
    uisThreadStop    = 0x0010,
    uisThreadRun     = 0x0020,
    uisHTMLEnabled   = 0x0040,
    uisHTMLDisabled  = 0x0080,
  //uisHasHeapPlugin = 0x0100,
    uisCommandLine   = 0x0200,  // debuggee launched with args
  //uisFindAgain     = 0x0400,
    uisCodeViews     = 0x0800,
    uisSymbolicView  = 0x1000,
    uisCheckUpdates  = 0x2000,
    uisDisable       = 0x8000,
    uisAny           = 0x7fff,

    uisAttachedThreadStop = (uisAttached | uisThreadStop)
};

#define STATE_MASK "smask"


#endif // STATES_H__FBE73FC3_8BEF_4C0C_A6AB_DDB0FCF96918
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
