#ifndef BREAKPOINT_ENABLER_H__F2639926_99D2_461E_80FF_D92C6774A8DF
#define BREAKPOINT_ENABLER_H__F2639926_99D2_461E_80FF_D92C6774A8DF
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include "zdk/breakpoint.h"

/**
 * Helper callback for enabling/disabling breakpoints
 */
CLASS BreakPointEnabler : public EnumCallback<volatile BreakPoint*>
{
public:
    BreakPointEnabler(bool onOff, int mask)
        : onOff_(onOff), mask_(mask)
    {}

protected:
    void notify(volatile BreakPoint* bpnt)
    {
        assert(bpnt);

        if ((bpnt->type() & mask_) == bpnt->type())
        {
            if (onOff_)
            {
                bpnt->enable();
            }
            else
            {
                bpnt->disable();
            }
        }
    }

private:
    bool onOff_;
    int  mask_;
};
#endif // BREAKPOINT_ENABLER_H__F2639926_99D2_461E_80FF_D92C6774A8DF
