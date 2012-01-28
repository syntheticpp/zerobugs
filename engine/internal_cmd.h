#ifndef INTERNAL_COMMAND_H__0B135951_4AC0_450A_9380_E7E10DD2A3FA
#define INTERNAL_COMMAND_H__0B135951_4AC0_450A_9380_E7E10DD2A3FA
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

#include <vector>
#include <string>
#include "zdk/command.h"
#include "zdk/zero.h"


class DebuggerShell;


typedef bool
(DebuggerShell::*CommandFun)(Thread*, const std::vector<std::string>& args);


// Auto-completion hook for readline().
// First parameter is currently typed word, second param is a
// list of possible matches to be filled out by the hook function
typedef void
(*AutoCompleteHook)(const char*, std::vector<std::string>&);



class ZDK_LOCAL InternalCommand : public ZObjectImpl<DebuggerCommand>
{
public:
    explicit InternalCommand(const Command&);
    virtual ~InternalCommand() throw();

protected:
    const char* name() const { return name_; }
    const char* help() const { return help_; }

    void auto_complete(const char*, EnumCallback<const char*>*) const;

    bool execute(Thread*, const char* const*, Unknown2*);

BEGIN_INTERFACE_MAP(InternalCommand)
    INTERFACE_ENTRY(DebuggerCommand)
END_INTERFACE_MAP()

private:
    const char* name_;
    const char* help_;

    CommandFun  fun_;
    AutoCompleteHook hook_;
};


struct ZDK_LOCAL Command
{
    const char* name;
    CommandFun  fun;
    AutoCompleteHook autoComplete;
    const char* help;

    //InternalCommand* construct() const
    //{ return new InternalCommand(*this); }
};


#endif // INTERNAL_COMMAND_H__0B135951_4AC0_450A_9380_E7E10DD2A3FA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
