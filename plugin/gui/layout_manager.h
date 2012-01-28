#ifndef LAYOUT_MANAGER_H__134F15CA_944B_4730_940F_20B8C9E1AF85
#define LAYOUT_MANAGER_H__134F15CA_944B_4730_940F_20B8C9E1AF85
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
#include <map>
#include <string>
#include "layout_strategy.h"


#define ALL_VISIBLE         "all"
#define TAB_LAYOUT          "tab"
#define LEFT_LAYOUT         "left"
#define SPLIT_TAB_LAYOUT    "split"

#define DEFAULT_STRATEGY    SPLIT_TAB_LAYOUT


/**
 * Controls the layout and appearance
 */
class ZDK_LOCAL LayoutManager
{
public:
    LayoutManager();

    LayoutStrategyPtr get_strategy(
        Debugger&,
        const std::string&, // strategy name
        Gtk::Container&     // container to organize
    ) const;

private:
    typedef LayoutStrategyPtr (*Creator)(Debugger&, Gtk::Container&);
    typedef std::map<std::string, Creator> StrategyMap;

    StrategyMap map_;
};


#endif // LAYOUT_MANAGER_H__134F15CA_944B_4730_940F_20B8C9E1AF85
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
