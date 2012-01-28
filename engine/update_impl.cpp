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

#include "update_impl.h"
#include "zdk/zero.h"


UpdateManager::UpdateManager(Debugger* debugger)
    : debugger_(debugger)
    , count_(0)
    , callback_(NULL)
{
}


size_t UpdateManager::check_for_updates(EnumCallback<Update*>* callback)
{
    count_ = 0;
    callback_ = callback;
    debugger_->enum_plugins(this);
    return count_;
}


void UpdateManager::notify(DebuggerPlugin* plugin)
{
    if (Updateable* updateable = interface_cast<Updateable*>(plugin))
    {
        count_ += updateable->check_for_updates(callback_);
    }
}

