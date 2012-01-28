#ifndef BREAKPOINT_REGISTRY_H__BBCA9C7E_3485_4525_96AB_94DE6B2EED2F
#define BREAKPOINT_REGISTRY_H__BBCA9C7E_3485_4525_96AB_94DE6B2EED2F
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
#include <map>
#include "generic/singleton.h"
#include "zdk/enum.h"
#include "zdk/weak_ptr.h"

class BreakPoint;
class BreakPointAction;


/**
 * Keep track of breakpoint actions, so that one
 * can refer to them by numberic IDs
 *
 * TODO better name: BreakPointActionRegistry?
 */
class ZDK_LOCAL BreakPointRegistry
    : public EnumCallback<volatile BreakPoint*>
    , private EnumCallback<BreakPointAction*>
{
    typedef WeakPtr<BreakPointAction> ActionPtr;

    // map actions to numbers
    typedef std::map<ActionPtr, size_t> ActionToNumber;

    // map numbers to breakpoint actions
    typedef std::map<size_t, ActionPtr> NumberToAction;

    ActionToNumber actionToNumber_;
    NumberToAction numberToAction_;

    volatile BreakPoint* brkpnt_;

public:
    BreakPointRegistry() : brkpnt_(NULL) { }
    ~BreakPointRegistry() throw() { }

    void notify(volatile BreakPoint*);
    void notify(BreakPointAction*);
};

typedef Singleton<BreakPointRegistry> TheBreakPointRegistry;

#endif // BREAKPOINT_REGISTRY_H__BBCA9C7E_3485_4525_96AB_94DE6B2EED2F

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
