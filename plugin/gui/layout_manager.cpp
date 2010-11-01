//
// $Id: layout_manager.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <assert.h>
#include <stdexcept>
#include "all_visible_strategy.h"
#include "left_layout_strategy.h"
#include "layout_manager.h"
#include "split_tab_layout_strategy.h"
#include "tab_layout_strategy.h"


using namespace std;


LayoutManager::LayoutManager()
{
    map_.insert(make_pair(ALL_VISIBLE, &AllVisibleStrategy::create));
    map_.insert(make_pair(TAB_LAYOUT, &TabLayoutStrategy::create));
    map_.insert(make_pair(LEFT_LAYOUT, &LeftLayoutStrategy::create));
    map_.insert(make_pair(SPLIT_TAB_LAYOUT, &SplitTabLayoutStrategy::create));
}



LayoutStrategyPtr LayoutManager::get_strategy
(
    Debugger& dbg,
    const string& name,
    Gtk::Container& container
) const
{
    LayoutStrategyPtr ptr;

    if (name == "default")
    {
        ptr = get_strategy(dbg, DEFAULT_STRATEGY, container);
    }
    else
    {
        StrategyMap::const_iterator i = map_.find(name);
        if (i == map_.end())
        {
            throw runtime_error("No such layout strategy: " + name);
        }
        ptr = (*i->second)(dbg, container);
    }
    return ptr;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
