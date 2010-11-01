//
// $Id: popup_list.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "gtkmm/color.h"
#include "gtkmm/connect.h"
#include "gtkmm/frame.h"
#include "gtkmm/flags.h"
#include "gtkmm/list.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/style.h"
#include "gtkmm/resize.h"
#include "gtkmm/widget.h"
#include "zdk/check_ptr.h"
#if !defined(GTKMM_2)
 #include <gtk--/main.h>
#endif
#include "dialog_box.h"
#include "popup_list.h"
#include "text_entry.h"


using namespace std;

static const int WIDTH = 400;
static const int HEIGHT = 250;

class PopupList::Impl : public Gtk::List
{
};


PopupList::PopupList()
    : Gtk::Window(Gtk_FLAG(WINDOW_POPUP))
    , impl_(new Impl)
    , textEntry_(0)
{
    Gtk::Frame* frame = manage(new Gtk::Frame);
    add(*frame);
    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow);
    frame->set_shadow_type(Gtk_FLAG(SHADOW_OUT));
    frame->add(*sw);
    Gtk_add_with_viewport(sw, *impl_);
    sw->set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));

    Gtk_CONNECT_0(impl_, selection_changed, this, &PopupList::on_selection);
    Gtk_set_size(this, WIDTH, HEIGHT);
}


void PopupList::set_items(const set<string>& items)
{
    impl_->items().clear();

    set<string>::const_iterator i = items.begin();
    for (; i != items.end(); ++i)
    {
        Gtk::ListItem* item = manage(new Gtk::ListItem(*i));
        impl_->add(*item);
    }
    impl_->show_all();
}


void PopupList::set_text_entry(TextEntry* entry)
{
    textEntry_ = entry;

    if (entry)
    {
        set_transient_for(entry->dialog());

    #ifdef GTKMM_2
        entry->dialog().remove_modal_grab();
    #else
        Gtk::Main::grab_remove(entry->dialog());
    #endif
    }
    set_position();
}


void PopupList::set_text_and_hide(const string& label)
{
    CHKPTR(textEntry_)->set_text(label);

    hide();
    textEntry_ = 0;
}


void PopupList::on_selection()
{
    if (textEntry_)
    {
        Gtk::List::SelectionList& sel = impl_->selection();

        if (!sel.empty())
        {
            string label = Gtk_list_item_get_text(*sel.begin());
            set_text_and_hide(label);
        }
    }
}



void PopupList::set_position()
{
    if (textEntry_)
    {
        int x = 0, y = 0, w = 0, h = 0;

        Gtk_WINDOW(textEntry_)->get_origin(x, y);
        Gtk_WINDOW(textEntry_)->get_size(w, h);

        if (gdk_screen_width() - (x + w) < WIDTH)
        {
            if (x >= WIDTH)
            {
                x -= WIDTH;
            }
            else
            {
                x = 0;
                y += h;
            }
        }
        else
        {
            x += w;
        }

        if (gdk_screen_height() - y < HEIGHT)
        {
            y -= (HEIGHT - h);
        }
        Gtk_move(this, x, y);
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
