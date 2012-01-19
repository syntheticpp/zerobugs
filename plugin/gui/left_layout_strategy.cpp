//
// $Id: left_layout_strategy.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "gtkmm/box.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/notebook.h"
#include "gtkmm/paned.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/resize.h"
#include "gtkmm/widget.h"
#include "zdk/properties.h"
#include "zdk/zero.h"
#include "left_layout_strategy.h"


using namespace boost;
using namespace Gtk;


LeftLayoutStrategy::LeftLayoutStrategy(Debugger& dbg, Container& c)
    : dbg_(dbg)
    , container_(c)
    , hpaned_(shared_ptr<HPaned>(new HPaned))
    , vpaned_(shared_ptr<VPaned>(new VPaned))
    , leftBook_(new Notebook_Adapt)
    , bottomBook_(new Notebook_Adapt)
    , varView_(NULL)
    , threadsViewIndex_(-1)
{
    leftBook_->set_tab_pos(Gtk_FLAG(POS_BOTTOM));
    bottomBook_->set_tab_pos(Gtk_FLAG(POS_BOTTOM));
    c1_ = Gtk_CONNECT_AFTER_0(
               bottomBook_,
               switch_page,
               this,
               &LeftLayoutStrategy::on_switch_page_bottom);
    c2_ = Gtk_CONNECT_AFTER_0(
               leftBook_,
               switch_page,
               this,
               &LeftLayoutStrategy::on_switch_page_right);

    Gtk_set_size(hpaned_, -1, 100);

    hpaned_->pack1(*leftBook_, resizeNo, shrinkNo);
    hpaned_->pack2(*vpaned_, resizeOk, shrinkNo);

    vpaned_->set_position(dbg_.properties()->get_word("code.height", 420));

    vpaned_->pack2(*bottomBook_, resizeNo, shrinkNo);

    hpaned_->set_gutter_size(10);
    vpaned_->set_gutter_size(10);

    Box* box = manage(new VBox);
    box->add(*hpaned_);

    container_.add(*box);

    Gtk_set_size(leftBook_, 250, -1);
    Gtk_set_size(bottomBook_, -1, 160);

    leftBook_->set_group(this);
    bottomBook_->set_group(this);
}


LeftLayoutStrategy::~LeftLayoutStrategy()
{
    try
    {
        c1_.disconnect();
        c2_.disconnect();

        save_geometry();
    }
    catch (...)
    {
    }
}


void LeftLayoutStrategy::save_geometry() const
{
    dbg_.properties()->set_word("left.width", hpaned_->get_position());
    dbg_.properties()->set_word("code.height", vpaned_->get_position());
}

void LeftLayoutStrategy::add_program_view(Widget& w)
{
    vpaned_->pack1(w, resizeNo, shrinkOk);
    hpaned_->set_position(dbg_.properties()->get_word("left.width", 250));
}


void LeftLayoutStrategy::add_registers_view(Widget& w)
{
    bottomBook_->append_page(w, "Registers");
}


void LeftLayoutStrategy::add_stack_view(Widget& w)
{
    ScrolledWindow* sw = manage(new ScrolledWindow);
    Gtk_add_with_viewport(sw, w);
    bottomBook_->append_page(*sw, "Stack Trace");
    bottomBook_->set_tab_detachable(*sw);
    bottomBook_->set_tab_reorderable(*sw);
}


void LeftLayoutStrategy::add_threads_view(Widget& w)
{
    threadsViewIndex_ = bottomBook_->append_page(w, "Threads");
    bottomBook_->set_tab_detachable(w);
    bottomBook_->set_tab_reorderable(w);
}


void LeftLayoutStrategy::show_threads_view()
{
    if (threadsViewIndex_ >= 0)
    {
        bottomBook_->set_current_page(threadsViewIndex_);
    }
}


void LeftLayoutStrategy::add_variables_view(Widget& w)
{
    leftBook_->append_page(w, "Local Variables");
    leftBook_->set_tab_detachable(w);
    leftBook_->set_tab_reorderable(w);
    assert(!varView_);

    varView_ = &w;
}


bool LeftLayoutStrategy::is_variables_view_visible() const
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
            pageNum = leftBook_->page_num(*varView_);
            if (pageNum >= 0)
            {
                result = leftBook_->get_current_page() == pageNum;
            }
        }
    }
    return result;
}


void LeftLayoutStrategy::on_switch_page_bottom(GtkNotebookPage*, guint)
{
    visibility_changed.emit(bottomBook_);
}


void LeftLayoutStrategy::on_switch_page_right(GtkNotebookPage*, guint)
{
    visibility_changed.emit(leftBook_);
}


void LeftLayoutStrategy::add_watches_view(Widget& w)
{
    leftBook_->append_page(w, "Watch Variables");
    leftBook_->set_tab_detachable(w);
    leftBook_->set_tab_reorderable(w);
}


void LeftLayoutStrategy::add_interpreter_box(Widget& w, const char* name)
{
    bottomBook_->append_page(w, name);
    bottomBook_->set_tab_reorderable(w);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
