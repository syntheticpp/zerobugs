#ifndef UPDATE_IMPL_H__96E56022_EECB_4CE0_A242_B095831E0CE9
#define UPDATE_IMPL_H__96E56022_EECB_4CE0_A242_B095831E0CE9
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: update_impl.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/update.h"
#include "zdk/zobject_impl.h"

struct Debugger;
struct DebuggerPlugin;


CLASS UpdateManager : public ZObjectImpl<Updateable>, EnumCallback<DebuggerPlugin*>
{
    Debugger*               debugger_;
    size_t                  count_;
    EnumCallback<Update*>*  callback_;

    void notify(DebuggerPlugin*);

public:
    explicit UpdateManager(Debugger*);

    size_t check_for_updates(EnumCallback<Update*>*);
};


#endif // UPDATE_IMPL_H__96E56022_EECB_4CE0_A242_B095831E0CE9
