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
#include "zdk/zero.h"
#include "breakpoint_registry.h"
#include "call.h"

using namespace std;


void BreakPointRegistry::notify(volatile BreakPoint* brkpnt)
{
    if (brkpnt)
    {
        brkpnt_ = brkpnt;
        brkpnt->enum_actions(NULL, this);
    }
}


void BreakPointRegistry::notify(BreakPointAction* action)
{
    assert(action);

    ActionToNumber::iterator i = actionToNumber_.find(action);

    if (i == actionToNumber_.end())
    {
        const size_t n = actionToNumber_.size() + 1;
        actionToNumber_.insert(make_pair(action, n));
        numberToAction_.insert(make_pair(n, action));

        // treat new breakpoint action as a new LOGICAL breakpoint
        WeakPtr<BreakPoint> brkpnt(const_cast<BreakPoint*>(brkpnt_));
        RefPtr<BreakPointAction> actPtr(action);
        call_<void>("on_new_breakpoint_action", n, brkpnt, actPtr);
    }
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
