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
#ifndef GTKMM_2
#include <cassert>
#include <iostream>
#include <gdk/gdkkeysyms.h>
#include "gtkmm/box.h"
#include "gtkmm/clist.h"
#include "gtkmm/ctree.h"
#include "gtkmm/entry.h"
#include "gtkmm/window.h"
#include "gtkmm/style.h"
#include "slot_macros.h"
#include "edit_in_place.h"


using namespace std;
using namespace Gtk;
using namespace SigC;


EditInPlace::EditInPlace(CList* list)
    : list_(list)
    , mapped_(false)
    , edit_(0)
    , editable_(list->columns().size())
    , nrow_(-1)
    , ncol_(-1)
{
}


int EditInPlace::on_map_event(GdkEventAny* event)
{
    if (mapped_)
    {
        return 0;
    }

    mapped_ = true;

    if (Window* w = get_list().get_toplevel())
    {
        w->configure_event.connect(
            slot(this, &EditInPlace::on_configure));
    }

    return 0;
}


void EditInPlace::on_select_row(
    int         nrow,
    int         ncol,
    GdkEvent*   event)
{
    // begin editing on double click
    if (event && event->type == GDK_2BUTTON_PRESS && editable_[ncol])
    {
        begin_edit(nrow, ncol, &event->button);
    }
}


void EditInPlace::adjust_to_cell(int& x, int& y)
{
    int nrow = 0;
    int ncol = 0;

    if (!get_list().get_selection_info(x, y, &nrow, &ncol))
    {
        return;
    }

    /* This is sort of a hack but I could not figure
       a better way of doing it. Decrement the X and Y
       coords until we get out of the current cell, to
       figure the coordinates of the north-west corner. */
    if (y <= get_list().get_row_height())
    {
        y = 0;
    }
    else
    {
        for (int r = nrow; r == nrow; --y)
        {
            if (!get_list().get_selection_info(x, y, &r, &ncol))
            {
                break;
            }
        }
    }
    ++y;

    for (int c = ncol; c == ncol; --x)
    {
        if (!get_list().get_selection_info(x, y, &nrow, &c))
        {
            break;
        }
    }
    ++x;
}


/**
 * Handle key press events
 */
BEGIN_SLOT_(int, EditInPlace::on_key, (GdkEventKey* key))
{
    assert(key);
    switch (key->keyval)
    {
    case GDK_Return:
        // attempt to commit the change
        {
            assert(edit_);

            string text = edit_->get_text();
            string old = get_list().cell(nrow_, ncol_).get_text();

            if (cell_edit(nrow_, ncol_, old, text))
            {
                get_list().cell(nrow_, ncol_).set_text(text);
            }
        }
        // fallthrough

    case GDK_Escape:
        finish_edit();
        break;
    }
}
END_SLOT_(0)


/* Abort editing when user clicks outside edit (entry) widget */
int EditInPlace::on_mouse_click(GdkEventButton*)
{
    finish_edit();
    return 0;
}


int EditInPlace::on_configure(GdkEventConfigure* event)
{
    if (w_.get() && event)
    {
        /* instead of moving the edit field around it,
           is simpler just to cancel editing altogether */
        finish_edit();
    }
    return 0;
}


void EditInPlace::finish_edit()
{
    w_.reset();
    edit_ = 0;

    nrow_ = ncol_ = -1;
 /*
    if (Gtk::Window* top = get_list().get_toplevel())
    {
        top->set_events(top->get_events() & ~GDK_VISIBILITY_NOTIFY_MASK);
    }
 */
}


void EditInPlace::begin_edit(int nrow, int ncol, GdkEventButton* event)
{
    w_.reset(new Window(GTK_WINDOW_POPUP));

    w_->set_policy(false, false, true);
    w_->set_border_width(1);
    w_->set_flags(GTK_CAN_FOCUS);
    w_->button_press_event.connect(slot(this, &EditInPlace::on_mouse_click));

    edit_ = manage(new Gtk::Entry());
    w_->add(*edit_);

    edit_->set_text(get_list().cell(nrow, ncol).get_text());
    edit_->set_position(0);
    edit_->key_press_event.connect(slot(this, &EditInPlace::on_key));

    int x = static_cast<int>(event->x);
    int y = static_cast<int>(event->y);

    double dx = event->x - event->x_root;
    double dy = event->y - event->y_root;

    adjust_to_cell(x, y);

    int width = -(x + 3);

    if (static_cast<size_t>(ncol) + 1 == get_list().columns().size())
    {
        width += get_list().width();
    }
    else
    {
        width += get_list().get_column_width(ncol);
    }
    w_->set_usize(width, -1);
    w_->set_uposition(
            x - static_cast<int>(dx),
            y - static_cast<int>(dy));

    edit_->set_flags(GTK_CAN_FOCUS);
    edit_->set_flags(GTK_CAN_DEFAULT);

    edit_->grab_focus();
    edit_->grab_default();

    w_->show_all();
    w_->set_modal(true);

    /* if (Gtk::Window* top = get_list().get_toplevel())
    {
        w_->set_transient_for(*top);
    } */

    edit_->set_flags(GTK_HAS_FOCUS);

    /* memorize row and column of the cell being edited */
    ncol_ = ncol;
    nrow_ = nrow;
}

#endif
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
