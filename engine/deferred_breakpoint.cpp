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

#include <iostream>
#include "breakpoint.h"
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"

using namespace std;


////////////////////////////////////////////////////////////////
DeferredBreakPoint::DeferredBreakPoint
(
    BreakPointManagerBase*  mgr,
    RefPtr<Thread>          thread,
    Type                    type,
    addr_t                  addr
)
  : BreakPointBase(thread, addr)
  , mgr_(mgr)
  , type_(type)
{
}


////////////////////////////////////////////////////////////////
DeferredBreakPoint::DeferredBreakPoint
(
    const DeferredBreakPoint&   other,
    BreakPointManagerBase*      mgr,
    Thread&                     thread
)
  : BreakPointBase(other, thread)
  , mgr_(mgr)
  , type_(DEFERRED)
{
}


////////////////////////////////////////////////////////////////
DeferredBreakPoint::~DeferredBreakPoint() throw()
{
    try
    {
        disable();
    }
    catch (const std::exception& e)
    {
        clog << e.what() << " in ~DeferredBreakpoint calling disable()\n";
    }
}


////////////////////////////////////////////////////////////////
BreakPointBase*
DeferredBreakPoint::clone(BreakPointManagerBase* base, RefPtr<Thread> thread)
{
#ifdef DEBUG
    clog << __func__ << " " << type() << " breakpoint at ";
    clog << hex << addr() << dec << endl;
#endif
    assert(thread);
    return thread ? new DeferredBreakPoint(*this, base, *thread) : 0;
}



////////////////////////////////////////////////////////////////
bool DeferredBreakPoint::do_enable(bool onOff) volatile
{
    return true;
}


////////////////////////////////////////////////////////////////
void DeferredBreakPoint::execute_actions(Thread*)
{
    throw logic_error("cannot execute actions on a deferred breakpoint");
}

