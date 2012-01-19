#ifndef LAYOUT_STRATEGY_H__92190B9D_2712_43E2_8235_72D4C7562F29
#define LAYOUT_STRATEGY_H__92190B9D_2712_43E2_8235_72D4C7562F29
//
// $Id: layout_strategy.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "gtkmm/base.h"
#include "gtkmm/notebook.h"
#include "gtkmm/signal.h"

class Debugger;

namespace Gtk
{
    class Container;
    class Widget;
}


/**
 * Controls how various views are shown inside of a container
 */
struct ZDK_LOCAL LayoutStrategy : public Gtk::Base
{
    virtual ~LayoutStrategy() { }

    SigC::Signal1<void, NotebookPtr> visibility_changed;

    virtual void add_program_view(Gtk::Widget&) = 0;

    virtual void add_registers_view(Gtk::Widget&) = 0;

    virtual void add_stack_view(Gtk::Widget&) = 0;

    virtual void add_variables_view(Gtk::Widget&) = 0;

    virtual void add_threads_view(Gtk::Widget&) = 0;
    virtual void show_threads_view() = 0;

    virtual void add_watches_view(Gtk::Widget&) = 0;

    virtual void add_interpreter_box(Gtk::Widget&, const char* name) = 0;

    virtual Gtk::Container& container() = 0;

    virtual bool is_variables_view_visible() const = 0;

    virtual void save_geometry() const = 0;
};


typedef boost::shared_ptr<LayoutStrategy> LayoutStrategyPtr;

#endif // LAYOUT_STRATEGY_H__92190B9D_2712_43E2_8235_72D4C7562F29
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
