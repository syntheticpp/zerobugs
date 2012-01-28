#ifndef ALL_VISIBLE_STRATEGY_H__0FB91FB4_82AF_4854_A399_B3F4F45E9BD8
#define ALL_VISIBLE_STRATEGY_H__0FB91FB4_82AF_4854_A399_B3F4F45E9BD8
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
#include <boost/utility.hpp>
#include "gtkmm/base.h"
#include "gtkmm/paned.h"
#include "layout_strategy.h"

//
// All views are visible at once
//
class ZDK_LOCAL AllVisibleStrategy : public LayoutStrategy
{
public:
    static LayoutStrategyPtr create(Debugger& dbg, Gtk::Container& c)
    {
        return LayoutStrategyPtr(new AllVisibleStrategy(dbg, c));
    }
    ~AllVisibleStrategy();

protected:
    AllVisibleStrategy(Debugger& dbg, Gtk::Container&);

    virtual void add_program_view(Gtk::Widget&);

    virtual void add_registers_view(Gtk::Widget&);

    virtual void add_stack_view(Gtk::Widget&);

    virtual void add_threads_view(Gtk::Widget&);
    virtual void show_threads_view();

    virtual void add_variables_view(Gtk::Widget&);
    virtual void add_watches_view(Gtk::Widget&);

    virtual void add_interpreter_box(Gtk::Widget&, const char* name);

    virtual Gtk::Container& container() { return container_; }

    virtual bool is_variables_view_visible() const { return true; }

    virtual void save_geometry() const;

private:
    Debugger& dbg_;
    Gtk::Container& container_;
    PanedPtr hpaned_;
    PanedPtr vpaned_;
    PanedPtr left_;
    PanedPtr right_;
};

#endif // ALL_VISIBLE_STRATEGY_H__0FB91FB4_82AF_4854_A399_B3F4F45E9BD8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
