#ifndef BREAKPOINT_UTIL_H__9675BF5E_0628_414F_A9DA_CD8BA7C48A43
#define BREAKPOINT_UTIL_H__9675BF5E_0628_414F_A9DA_CD8BA7C48A43
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
#include <string>
#include "zdk/export.h"
#include "zdk/platform.h"

struct BreakPoint;
struct Debugger;
struct Thread;

using Platform::addr_t;


bool ZDK_LOCAL has_switchable_actions(volatile BreakPoint&);

bool ZDK_LOCAL has_disabled_actions(volatile BreakPoint&);

bool ZDK_LOCAL has_enabled_actions(volatile BreakPoint&);

bool ZDK_LOCAL enable_actions(volatile BreakPoint&);

bool ZDK_LOCAL enable_user_actions(volatile BreakPoint&);

bool ZDK_LOCAL has_conditional_actions(Debugger&, addr_t);

/**
 * Determine whether there are any enabled breakpoint
 * actions set at the specified address. If no thread is
 * given, global breakpoints are looked up
 */
bool ZDK_LOCAL has_enabled_user_breakpoint_actions(Debugger&, addr_t, Thread* = 0);

/**
 * Determine if there are any disabled breakpoint
 * actions set at the specified address.
 */
bool ZDK_LOCAL has_disabled_user_breakpoint_actions(Debugger&, addr_t, Thread* = 0);

/**
 * Enable all the global breakpoint actions at address.
 * If there are no disabled breakpoint actions, nothin happens.
 */
void ZDK_LOCAL enable_breakpoint_actions(Debugger&, addr_t);

void ZDK_LOCAL disable_breakpoint_actions(Debugger&, addr_t);

void ZDK_LOCAL enable_user_breakpoint_actions(Debugger&, addr_t, Thread* = 0);

void ZDK_LOCAL disable_user_breakpoint_actions(Debugger&, addr_t, Thread* = 0);

void ZDK_LOCAL set_breakpoint_conditions
(
    Debugger&           dbg,
    addr_t              addr,
    const std::string&  expr,
    unsigned long       activationCount,
    bool                autoReset
);

#endif // BREAKPOINT_UTIL_H__9675BF5E_0628_414F_A9DA_CD8BA7C48A43
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
