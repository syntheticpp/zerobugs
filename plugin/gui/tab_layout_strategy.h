#ifndef TAB_LAYOUT_STRATEGY_H__323AA594_F124_4D6F_AFBD_63AABF0BB5FC
#define TAB_LAYOUT_STRATEGY_H__323AA594_F124_4D6F_AFBD_63AABF0BB5FC
//
// $Id: tab_layout_strategy.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "gtkmm/base.h"
#include "gtkmm/notebook.h"
#include "gtkmm/paned.h"
#include <sigc++/slot.h>
#include "layout_strategy.h"


namespace Gtk
{
    class Box;
}


class ZDK_LOCAL TabLayoutStrategy : public LayoutStrategy
{
public:
    static LayoutStrategyPtr create(Debugger& dbg, Gtk::Container& c)
    {
        return LayoutStrategyPtr(new TabLayoutStrategy(dbg, c));
    }

    ~TabLayoutStrategy();

protected:
    TabLayoutStrategy(Debugger&, Gtk::Container&);

    virtual void add_program_view(Gtk::Widget&);

    virtual void add_registers_view(Gtk::Widget&);

    virtual void add_stack_view(Gtk::Widget&);

    virtual void add_threads_view(Gtk::Widget&);

    virtual void add_variables_view(Gtk::Widget&);

    virtual void add_watches_view(Gtk::Widget&);

    virtual void add_interpreter_box(Gtk::Widget&, const char* name);

    virtual Gtk::Container& container() { return container_; }

    virtual bool is_variables_view_visible() const;

    void on_switch_page_bottom(GtkNotebookPage*, guint pageNum);
    void on_switch_page_right(GtkNotebookPage*, guint pageNum);

protected:
    Debugger& dbg_;
    Gtk::Container& container_;
    SigC::Connection c1_, c2_;  // on_switch_page_bottom,
                                // on_switch_page_right
    PanedPtr hpaned_;
    PanedPtr vpaned_;
    NotebookPtr rightBook_;
    NotebookPtr bottomBook_;
    Gtk::Widget* varView_;
};


#endif // TAB_LAYOUT_STRATEGY_H__323AA594_F124_4D6F_AFBD_63AABF0BB5FC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
