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
#include "zdk/log.h"
#include "zdk/zero.h"
#include "gtkmm/box.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/notebook.h"
#include "gtkmm/paned.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/resize.h"
#include "gtkmm/widget.h"
#include "zdk/properties.h"
#include "tab_layout_strategy.h"
#include <iostream>

using namespace boost;
using namespace Gtk;


TabLayoutStrategy::TabLayoutStrategy(Debugger& dbg, Container& c)
    : dbg_(dbg)
    , container_(c)
    , hpaned_(shared_ptr<HPaned>(new HPaned))
    , vpaned_(shared_ptr<VPaned>(new VPaned))
    , rightBook_(new Notebook_Adapt)
    , bottomBook_(new Notebook_Adapt)
    , varView_(NULL)
    , threadsViewIndex_(-1)
{
    rightBook_->set_tab_pos(Gtk_FLAG(POS_BOTTOM));
    rightBook_->set_scrollable(true);
    bottomBook_->set_tab_pos(Gtk_FLAG(POS_BOTTOM));
    c1_ = Gtk_CONNECT_AFTER_0(
               bottomBook_,
               switch_page,
               this,
               &TabLayoutStrategy::on_switch_page_bottom);
    c2_ = Gtk_CONNECT_AFTER_0(
               rightBook_,
               switch_page,
               this,
               &TabLayoutStrategy::on_switch_page_right);

    Gtk_set_size(hpaned_, -1, 100);
    vpaned_->pack1(*hpaned_, resizeOk, shrinkNo);
    vpaned_->pack2(*bottomBook_, resizeNo, shrinkNo);
    vpaned_->set_position(dbg_.properties()->get_word("code.height", 420));

    hpaned_->pack2(*rightBook_, resizeNo, shrinkNo);
    hpaned_->set_gutter_size(10);
    vpaned_->set_gutter_size(10);

    Box* box = manage(new VBox);
    box->add(*vpaned_);

    container_.add(*box);

    Gtk_set_size(rightBook_, 100, -1);
    Gtk_set_size(bottomBook_, -1, 160);

    rightBook_->set_group(this);
    bottomBook_->set_group(this);
}


TabLayoutStrategy::~TabLayoutStrategy()
{
    c1_.disconnect();
    c2_.disconnect();

    try
    {
        save_geometry();
    }
    catch (...)
    {
    }
}


void TabLayoutStrategy::save_geometry() const
{
    if (dbg_.properties()->get_word("window.maximized", 0))
    {
        dbgout(0) << __func__ << ": is maximized, bailing out." << std::endl;
        return;
    }
    dbg_.properties()->set_word("code.width", hpaned_->get_position());
    dbg_.properties()->set_word("code.height", vpaned_->get_position());
}


void TabLayoutStrategy::add_program_view(Widget& w)
{
    hpaned_->pack1(w, resizeNo, shrinkNo);
    hpaned_->set_position(dbg_.properties()->get_word("code.width", 550));
}


void TabLayoutStrategy::add_registers_view(Widget& w)
{
    rightBook_->append_page(w, "Registers");
}


void TabLayoutStrategy::add_stack_view(Widget& w)
{
    ScrolledWindow* sw = manage(new ScrolledWindow);
    Gtk_add_with_viewport(sw, w);
    bottomBook_->append_page(*sw, "Stack Trace");
    bottomBook_->set_tab_detachable(*sw);
    bottomBook_->set_tab_reorderable(*sw);
}


void TabLayoutStrategy::add_threads_view(Widget& w)
{
    threadsViewIndex_ = rightBook_->append_page(w, "Threads");

    rightBook_->set_tab_detachable(w);
    rightBook_->set_tab_reorderable(w);
}


void TabLayoutStrategy::show_threads_view()
{
    if (threadsViewIndex_ >= 0)
    {
        rightBook_->set_current_page(threadsViewIndex_);
    }
}


void TabLayoutStrategy::add_variables_view(Widget& w)
{
    bottomBook_->append_page(w, "Local Variables");
    bottomBook_->set_tab_detachable(w);
    bottomBook_->set_tab_reorderable(w);
    assert(!varView_);

    varView_ = &w;
}


bool TabLayoutStrategy::is_variables_view_visible() const
{
    bool result = false;
    if (varView_)
    {
        int pageNum = bottomBook_->page_num(*varView_);
        if (pageNum >= 0)
        {
            result = bottomBook_->get_current_page() == pageNum;
        }
        else
        {
            pageNum = rightBook_->page_num(*varView_);
            if (pageNum >= 0)
            {
                result = rightBook_->get_current_page() == pageNum;
            }
        }
    }
    return result;
}


void TabLayoutStrategy::on_switch_page_bottom(GtkNotebookPage*, guint)
{
    visibility_changed.emit(bottomBook_);
}


void TabLayoutStrategy::on_switch_page_right(GtkNotebookPage*, guint)
{
    visibility_changed.emit(rightBook_);
}


void TabLayoutStrategy::add_watches_view(Widget& w)
{
    bottomBook_->append_page(w, "Watch Variables");
    bottomBook_->set_tab_detachable(w);
    bottomBook_->set_tab_reorderable(w);
}


void TabLayoutStrategy::add_interpreter_box(Widget& w, const char* name)
{
    bottomBook_->append_page(w, name);
    bottomBook_->set_tab_reorderable(w);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
