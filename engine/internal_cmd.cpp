//
// $Id: internal_cmd.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iostream>
#include "debugger_shell.h"
#include "internal_cmd.h"


using namespace std;


////////////////////////////////////////////////////////////////
InternalCommand::InternalCommand(const Command& cmd)
  : name_(cmd.name)
  , help_(cmd.help)
  , fun_(cmd.fun)
  , hook_(cmd.autoComplete)
{
}

////////////////////////////////////////////////////////////////
InternalCommand::~InternalCommand() throw()
{
}

////////////////////////////////////////////////////////////////
void InternalCommand::auto_complete(
    const char* word,
    EnumCallback<const char*>* callback) const
{
    assert(word);
    assert(callback);

    if (hook_)
    {
        vector<string> matches;

        (hook_)(word, matches);

        vector<string>::const_iterator i(matches.begin());
        for (; i != matches.end(); ++i)
        {
            callback->notify(i->c_str());
        }
    }
}

////////////////////////////////////////////////////////////////
bool InternalCommand::execute
(
    Thread* thread,
    const char* const* argv,
    Unknown2* // reserved for future extensions
)
{
    vector<string> args;

    while (argv && *argv)
    {
        args.push_back(*argv++);
    }

    return (DebuggerShell::instance().*fun_)(thread, args);
}

// Copyright (c) 2004, 2006 Cristian L. Vlasceanu

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
