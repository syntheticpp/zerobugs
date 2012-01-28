#ifndef SPLIT_TAB_LAYOUT_STRATEGY_H__525D79E3_3FA3_4C19_8C66_F19B2580C7FF
#define SPLIT_TAB_LAYOUT_STRATEGY_H__525D79E3_3FA3_4C19_8C66_F19B2580C7FF
//
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
//
#include "tab_layout_strategy.h"


class ZDK_LOCAL SplitTabLayoutStrategy : public TabLayoutStrategy
{
public:
    static LayoutStrategyPtr create(Debugger& dbg, Gtk::Container& c)
    {
        return LayoutStrategyPtr(new SplitTabLayoutStrategy(dbg, c));
    }

    ~SplitTabLayoutStrategy();

protected:
    SplitTabLayoutStrategy(Debugger&, Gtk::Container&);

    virtual void add_variables_view(Gtk::Widget&);

    virtual void add_watches_view(Gtk::Widget&);
    virtual void on_switch_page_bottom(GtkNotebookPage*, guint pageNum);

    virtual bool is_variables_view_visible() const;

private:
    PanedPtr splitPane_;
    NotebookPtr varBook_;
};


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
#endif // SPLIT_TAB_LAYOUT_STRATEGY_H__525D79E3_3FA3_4C19_8C66_F19B2580C7FF
