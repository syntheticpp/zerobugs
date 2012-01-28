#ifndef COMMAND_H__B5454532_EE0E_4B8C_9D19_99AEAE279129
#define COMMAND_H__B5454532_EE0E_4B8C_9D19_99AEAE279129
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

#include "zdk/enum.h"
#include "zdk/zobject.h"

struct Thread;


/**
 * Interface for plug-ins that want to hook their own set
 * of commands into the engine's command line user interface.
 */
DECLARE_ZDK_INTERFACE_(DebuggerCommand, ZObject)
{
    DECLARE_UUID("9a8ebda5-0152-4398-9d08-e5af1244ccc4")

    virtual const char* name() const = 0;
    virtual const char* help() const = 0;

    virtual void auto_complete(
        const char*,
        EnumCallback<const char*>*) const = 0;

    /**
     * @return true to resume target
     */
    virtual bool execute(
        Thread*,
        const char* const* argv, // NULL-ended list of args
        Unknown2* = 0) = 0;
};


DECLARE_ZDK_INTERFACE_(CommandCenter, struct Unknown)
{
    DECLARE_UUID("54a39829-f1e9-41bc-91e1-6279df22d7cc")

    virtual void add_command(DebuggerCommand*) = 0;

    virtual void enable_command(DebuggerCommand*, bool = true) = 0;
};

#endif // COMMAND_H__B5454532_EE0E_4B8C_9D19_99AEAE279129
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
