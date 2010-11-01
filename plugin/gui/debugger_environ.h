#ifndef DEBUGGER_ENVIRON_H__5A113147_B976_41E6_A58B_BCD8BEDF78DC
#define DEBUGGER_ENVIRON_H__5A113147_B976_41E6_A58B_BCD8BEDF78DC
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: debugger_environ.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "environment.h"
#include "zdk/zero.h"


CLASS DebuggerEnvironment : public Environment
{
    Debugger& debugger_;

public:
    explicit DebuggerEnvironment(Debugger& debugger)
        : debugger_(debugger)
        { }
    const char* const* get(bool reset)
        { return debugger_.environment(reset); }
    void set(const char* const* env)
        { debugger_.set_environment(env); }
};
#endif // DEBUGGER_ENVIRON_H__5A113147_B976_41E6_A58B_BCD8BEDF78DC
