//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: proxy-plugin.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "engine/debugger_base.h"
#include "proxy-plugin.h"
#include "proxy-target.h"
#include "target/target_factory.h"


using namespace std;


static RefPtr<Target> create_target(debugger_type& debugger)
{
    return new ProxyTarget(debugger, __WORDSIZE);
}


ProxyPlugin::ProxyPlugin() : dbg_(NULL)
{
}


ProxyPlugin::~ProxyPlugin() throw()
{
}


bool ProxyPlugin::initialize(Debugger* dbg, int* argc, char*** argv)
{
    dbg_ = dbg;
    bool result = false;

    if (dbg)
    {
        dbg->set_options(dbg->options() | Debugger::OPT_ACCEPT_TARGET_PARAM);

        DebuggerBase& base = static_cast<DebuggerBase&>(*dbg);
        result = base.target_factory().register_target(
            TargetFactory::Linux,
            __WORDSIZE,
            true,  // live
            "remote",
            create_target,
            true);
    }
    return result;
}




void ProxyPlugin::start()
{
    DebuggerBase& base = static_cast<DebuggerBase&>(*dbg_);

    if (!base.target_factory().register_target(
            TargetFactory::Linux,
            __WORDSIZE,
           true, // live target
           "remote",
            create_target,
           true))// override
    {
        throw runtime_error("Failed to register remote debugging target");
    }
}


void ProxyPlugin::shutdown()
{
}


void ProxyPlugin::release()
{
    delete this;
}

