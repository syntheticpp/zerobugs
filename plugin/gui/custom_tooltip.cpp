//
// $Id: custom_tooltip.cpp 720 2010-10-28 06:37:54Z root $
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
#include <iostream>
#include <vector>
#include "generic/temporary.h"
#include "gtkmm/clist.h"
#include "gtkmm/connect.h"
#include "gtkmm/ctree.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "gtkmm/resize.h"
#include "gtkmm/style.h"
#include "gtkmm/widget.h"
#include "gtkmm/window.h"
#include "custom_tooltip.h"



CustomToolTip::CustomToolTip(Gtk::Widget* w, GetTextAtPointer fn)
    : widget_(w)
    , getTextAtPointer_(fn)
    , hrow_(INIT_ROW_HANDLE)
    , hcol_(-1)
    , drawing_(false)
{
    assert(w);
    assert(fn);

    w->add_events(Gdk_FLAG(POINTER_MOTION_MASK)
                | Gdk_FLAG(ENTER_NOTIFY_MASK)
                | Gdk_FLAG(LEAVE_NOTIFY_MASK)
             /* | Gdk_FLAG(BUTTON_PRESS_MASK) */);
}


CustomToolTip::~CustomToolTip()
{
}


void CustomToolTip::reset_tooltip()
{
    tooltip_.reset();
}


#if !GTKMM_2
void CustomToolTip::position_tooltip(int x, int y)
{
    assert(tooltip_.get());
    // show the tooltip slightly off from the mouse
    x += 10;
    y += 10;

    // make sure that the tooltip is fully visible,
    // by forcing its coordinates inside the screen
    Gtk_adjust_coords_for_visibility(tooltip_, x, y);
    Gtk_move(tooltip_.get(), x, y);
}
#endif


int CustomToolTip::on_pointer_motion(GdkEventMotion* event)
{
#if !GTKMM_2
    // silence off compiler warnings about converting doubles to ints
    const int x_root = static_cast<int>(event->x_root);
    const int y_root = static_cast<int>(event->y_root);

    if (tooltip_.get())
    {
        position_tooltip(x_root, y_root);
    }
#endif
    assert(widget_);
    std::string text;

    if (!(*getTextAtPointer_)(*widget_, event->x, event->y, hrow_, hcol_, text))
    {
        return 0;
    }
#if (GTKMM_2)
    widget_->set_tooltip_text(text);
#else
    // Gtk-1.2 gunk
    if (text.empty())
    {
        tooltip_.reset();
        hrow_ = INIT_ROW_HANDLE;
        hcol_ = -1;
    }
    else
    {
        tooltip_.reset(new Gtk::Window(Gtk_FLAG(WINDOW_POPUP)));

        assert(tooltip_.get());
        tooltip_->set_name("tooltip");
        tooltip_->set_border_width(3);

        Gtk_set_resizable(tooltip_, false);

        tooltip_->set_app_paintable(true);
        Gtk_CONNECT_0(tooltip_, expose_event, this, &CustomToolTip::on_paint_custom_tip);

        Gtk::Label* label = manage(new Gtk::Label(text, .0));
        tooltip_->add(*label);

        label->set_line_wrap(false);
        label->set_justify(Gtk_FLAG(JUSTIFY_LEFT));

        StylePtr style = CHKPTR(tooltip_->get_style())->copy();
        // fixme: get the background color from the current theme
        style->set_bg(Gtk_FLAG(STATE_NORMAL), Gdk_Color("lightyellow"));
        style->set_fg(Gtk_FLAG(STATE_NORMAL), Gdk_Color("black"));
        Gtk_set_style(tooltip_, style);

        //move it out of sight, initially
        Gtk_move(tooltip_.get(), gdk_screen_width(), gdk_screen_height());
        assert(tooltip_.get());

        tooltip_->show_all();
        position_tooltip(x_root, y_root);
    }
#endif
    return 0;
}


int CustomToolTip::on_pointer_leave(GdkEventCrossing*)
{
#if GTKMM_2
    widget_->set_has_tooltip(false);
#else
    tooltip_.reset();

    hrow_ = INIT_ROW_HANDLE;
    hcol_ = -1;
#endif
    return 0;
}


int CustomToolTip::on_paint_custom_tip(GdkEventExpose* event)
{
    if (drawing_)
    {
        return 0;
    }
    assert(event);
    assert(widget_); // otherwise how could we get this event?

    if (event->count == 0 && tooltip_.get())
    {
        Temporary<bool> setFlag(drawing_, true);

        assert(tooltip_->get_style());
        const GdkRectangle& r = event->area;

        gtk_paint_flat_box(GTKOBJ(tooltip_->get_style()),
                           event->window,
                           GTK_STATE_NORMAL,
                           GTK_SHADOW_OUT,
                           NULL,
                           GTK_WIDGET(GTKOBJ(tooltip_)),
                           "tooltip",
                           r.x, r.y, r.width, r.height);
//#ifdef GTKMM_2
//        std::vector<Gtk::Widget*> children = tooltip_->get_children();
//        gtk_widget_draw(GTKOBJ(children[0]), NULL);
//#endif
    }
    return 0;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
