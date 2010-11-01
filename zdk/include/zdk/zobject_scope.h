#ifndef ZOBJECT_SCOPE_H__AED3E803_0802_40F9_A627_C2901ADD2721
#define ZOBJECT_SCOPE_H__AED3E803_0802_40F9_A627_C2901ADD2721

// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: zobject_scope.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zobject.h"
#include <vector>

class ZDK_LOCAL ZObjectScope : public ZObjectManager
{
BEGIN_INTERFACE_MAP(ZObjectScope)
    INTERFACE_ENTRY(ZObjectManager)
END_INTERFACE_MAP()

    std::vector<RefPtr<ZObject> > objects_;

    void manage(ZObject* obj) { objects_.push_back(obj); }
};
#endif // ZOBJECT_SCOPE_H__AED3E803_0802_40F9_A627_C2901ADD2721
