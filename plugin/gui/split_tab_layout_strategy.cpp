//
// $Id: split_tab_layout_strategy.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/notebook.h"
#include "gtkmm/paned.h"
#include "gtkmm/resize.h"
#include "gtkmm/widget.h"
#include "zdk/properties.h"
#include "zdk/zero.h"
#include "split_tab_layout_strategy.h"


using namespace boost;
using namespace Gtk;


SplitTabLayoutStrategy::SplitTabLayoutStrategy(Debugger& dbg, Container& c)
    : TabLayoutStrategy(dbg, c)
    , splitPane_(shared_ptr<HPaned>(new HPaned))
    , varBook_(new Notebook_Adapt)
{
    vpaned_->remove(*bottomBook_);
    vpaned_->pack2(*splitPane_, resizeNo, shrinkNo);
    splitPane_->add1(*varBook_);
    splitPane_->add2(*bottomBook_);
    splitPane_->set_gutter_size(10);
    splitPane_->set_position(dbg_.properties()->get_word("stack.width", 550));
    varBook_->set_tab_pos(Gtk_FLAG(POS_BOTTOM));
}


SplitTabLayoutStrategy::~SplitTabLayoutStrategy()
{
    c1_.disconnect();
    c2_.disconnect();

    try
    {
        dbg_.properties()->set_word("stack.width", splitPane_->get_position());
    }
    catch (...)
    {
    }
}


void SplitTabLayoutStrategy::add_variables_view(Widget& w)
{
    varBook_->append_page(w, "Local Variables");
    varBook_->set_tab_detachable(w);
    varBook_->set_tab_reorderable(w);
    assert(!varView_);

    varView_ = &w;
}


void SplitTabLayoutStrategy::add_watches_view(Widget& w)
{
    varBook_->append_page(w, "Watch Variables");
    varBook_->set_tab_detachable(w);
    varBook_->set_tab_reorderable(w);
}


void SplitTabLayoutStrategy::on_switch_page_bottom(GtkNotebookPage*, guint)
{
    visibility_changed.emit(varBook_);
    visibility_changed.emit(bottomBook_);
}


bool SplitTabLayoutStrategy::is_variables_view_visible() const
{
    bool result = false;
    if (varView_)
    {
        int pageNum = varBook_->page_num(*varView_);
        if (pageNum >= 0)
        {
            result = varBook_->get_current_page() == pageNum;
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
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

