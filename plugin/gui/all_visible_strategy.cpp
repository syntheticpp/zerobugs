//
// $Id: all_visible_strategy.cpp 720 2010-10-28 06:37:54Z root $
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
#include "gtkmm/box.h"
#include "gtkmm/paned.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "zdk/check_ptr.h"
#include "zdk/properties.h"
#include "zdk/zero.h"
#include "all_visible_strategy.h"

using namespace boost;
using namespace Gtk;

const size_t PANED_GUTTER_SIZE = 8;



AllVisibleStrategy::AllVisibleStrategy(Debugger& dbg, Container& c)
    : dbg_(dbg)
    , container_(c)
    , hpaned_(shared_ptr<HPaned>(new HPaned))
    , vpaned_(shared_ptr<VPaned>(new VPaned))
    , left_(shared_ptr<VPaned>(new VPaned))
    , right_(shared_ptr<VPaned>(new VPaned))
{
    vpaned_->pack2(*hpaned_, resizeOk, shrinkNo);
    vpaned_->set_position(dbg_.properties()->get_word("locals.height", 120));

    hpaned_->pack1(*left_, resizeOk, shrinkNo);
    hpaned_->pack2(*right_, resizeOk, shrinkNo);
    hpaned_->set_position(dbg_.properties()->get_word("code.width", 460));
    left_->set_position(120);
    right_->set_position(120);

    Gtk_set_size(right_, 180, -1);

    Gtk::Box* box = manage(new VBox);
    box->add(*vpaned_);
    container_.add(*box);

    hpaned_->set_gutter_size(PANED_GUTTER_SIZE);
    vpaned_->set_gutter_size(PANED_GUTTER_SIZE);
    left_->set_gutter_size(PANED_GUTTER_SIZE);
    right_->set_gutter_size(PANED_GUTTER_SIZE);
}


AllVisibleStrategy::~AllVisibleStrategy()
{
    try
    {
        save_geometry();
    }
    catch (...)
    {
    }
}


void AllVisibleStrategy::save_geometry() const
{
    dbg_.properties()->set_word("locals.height", vpaned_->get_position());
    dbg_.properties()->set_word("code.width", hpaned_->get_position());
}

void AllVisibleStrategy::add_variables_view(Widget& w)
{
    CHKPTR(vpaned_)->pack1(w, resizeOk, shrinkNo);
}


void AllVisibleStrategy::add_program_view(Widget& w)
{
    CHKPTR(left_)->pack2(w, resizeOk, shrinkNo);
}


void AllVisibleStrategy::add_registers_view(Widget& w)
{
    CHKPTR(right_)->pack2(w, resizeNo, shrinkNo);
}


void AllVisibleStrategy::add_stack_view(Widget& w)
{
    ScrolledWindow* sw = manage(new ScrolledWindow);
    Gtk_add_with_viewport(sw, w);

    CHKPTR(left_)->pack1(*sw, resizeOk, shrinkNo);
}


void AllVisibleStrategy::add_threads_view(Widget& w)
{
    CHKPTR(right_)->pack1(w, resizeNo, shrinkNo);
}

void AllVisibleStrategy::show_threads_view()
{
}


void AllVisibleStrategy::add_watches_view(Widget&)
{
}


void AllVisibleStrategy::add_interpreter_box(Widget&, const char*)
{
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
