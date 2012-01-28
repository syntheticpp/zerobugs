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
#include <gdk/gdkkeysyms.h>     // GDK_VoidSymbol
#include "gtkmm/accelgroup.h"
#include "gtkmm/add_accel.h"
#include "gtkmm/button.h"
#include "gtkmm/connect.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "gtkmm/main.h"
#include "gtkmm/resize.h"
#include "gtkmm/widget.h"
#include "dialog_box.h"

using namespace std;

Gtk::Dialog* DialogBox::active_ = 0;


DialogBox::DialogBox(ButtonID btns, const char* title)
  : bbox_(manage(new Gtk::HButtonBox))
  , okBtn_(0)
  , result_(btn_cancel)
  , oldActive_(0)
{
    get_action_area()->pack_start(*bbox_);

    if (!title)
    {
        title = "Zero";
    }
    set_title(title);

    static struct
    {
        const char*name;
    }
    btn_face [] =
    {
#ifdef GTKMM_2
        { GTK_STOCK_OK      },
        { GTK_STOCK_YES     },
        { "Yes All"         },
        { GTK_STOCK_NO      },
        { GTK_STOCK_CANCEL  },
        { GTK_STOCK_CLOSE   },
#else
        { "_OK",     },
        { "_Yes",    },
        { "Yes _All",},
        { "_No",     },
        { "_Cancel"  },
        { "_Close"   },
#endif
    };

    static const unsigned NUM_BTNS = sizeof(btn_face)/sizeof(btn_face[0]);

    bbox_->set_layout(Gtk_FLAG(BUTTONBOX_SPREAD));
    bbox_->set_spacing(0);

    Gtk::Button* btn = 0;

    for (size_t i(0); i != NUM_BTNS; ++i)
    {
        ButtonID id = static_cast<ButtonID>(1 << i);
        if (btns & id)
        {
            btn = manage(new Gtk::Button(Gtk_STOCK_ID(btn_face[i].name)));
            if (!btn)
            {
                continue;
            }
            bbox_->pack_start(*btn);

            Gtk_CONNECT_1(btn, clicked, this, &DialogBox::on_button, id);
            btn->set_flags(Gtk_FLAG(CAN_DEFAULT) | Gtk_FLAG(CAN_FOCUS));

            if (id == btn_ok)
            {
                okBtn_ = btn;
                Gtk_ADD_ACCEL(*btn, "clicked", get_accel_group(), GDK_Return,
                              Gdk_FLAG(MOD_NONE), Gtk_FLAG(ACCEL_LOCKED));
            }
            else if (id == btn_cancel || id == btn_close)
            {
                Gtk_ADD_ACCEL(*btn, "clicked", get_accel_group(), GDK_Escape,
                              Gdk_FLAG(MOD_NONE), Gtk_FLAG(ACCEL_LOCKED));
            }
            add_button_accelerator(*btn);
        }
    }
    // last added button is default
    if (btn)
    {
        btn->grab_default();
    }
    Gtk_set_resizable(this, false);
}


DialogBox::~DialogBox()
{
    if (active_ == this)
    {
        swap(active_, oldActive_);
    }
}


Gtk::Button*
DialogBox::add_button(const char* label, Gtk::ButtonBox* bbox)
{
    Gtk::Button* btn = manage(new Gtk::Button(label));
    bbox->add(*btn);
    btn->set_flags(Gtk_FLAG(CAN_DEFAULT));
    add_button_accelerator(*btn);
    return btn;
}


Gtk::Button*
DialogBox::add_button(const Gtk::StockID& id, Gtk::ButtonBox* bbox)
{
    Gtk::Button* btn = manage(new Gtk::Button(id));
    bbox->add(*btn);
    btn->set_flags(Gtk_FLAG(CAN_DEFAULT));
    return btn;
}


void DialogBox::add_button_accelerator(Gtk::Button& btn, int accel)
{
    if (!get_accel_group())
    {
        return;
    }
    if (accel == 0)
    {
        if (Gtk::Label* lbl = dynamic_cast<Gtk::Label*>(btn.get_child()))
        {
            accel = Gtk_get_mnemonic_keyval(lbl);
        }
    }
    if (accel != GDK_VoidSymbol && accel > 0)
    {
        Gtk_ADD_ACCEL(btn, "clicked", get_accel_group(), accel,
                  Gdk_FLAG(MOD1_MASK), Gtk_FLAG(ACCEL_LOCKED));
#if !GTKMM_2
        Gtk_ADD_ACCEL(btn, "clicked", get_accel_group(), accel,
                  Gdk_FLAG(MOD_NONE), Gtk_FLAG(ACCEL_LOCKED));
#endif
    }
}


event_result_t DialogBox::on_delete_event(GdkEventAny* event)
{
    result_ = btn_cancel;
    close_dialog();
    return 1;
}


void DialogBox::on_button(ButtonID btn)
{
    result_ = btn;
    close_dialog();
}


DialogBox::ButtonID DialogBox::run(const Gtk::Widget* w, bool modal)
{
    if (w)
    {
        center(*w);
    }
    else
    {
        set_position(Gtk_FLAG(WIN_POS_MOUSE));
    }
    show_all();
    set_modal(modal);

    oldActive_ = this;
    swap(active_, oldActive_);

    Gtk::Main::run();
    return result_;
}


DialogBox::ButtonID DialogBox::run(int x, int y)
{
    set_uposition(x, y);
    show_all();
    set_modal(true);

    Gtk::Main::run();
    return result_;
}



void DialogBox::close_dialog()
{
    swap(active_, oldActive_);
    dialog_closed_event();
    Gtk::Main::quit();
}


void DialogBox::center(gint x, gint y, gint w, gint h)
{
    if (!is_realized())
    {
        set_uposition(gdk_screen_width(), gdk_screen_height());
        show_all();
    }

    int dw = 0, dh = 0;
    Gtk_WINDOW(this)->get_size(dw, dh);

    if (w == -1)
    {
        w = x;
    }
    if (h == -1)
    {
        h = y;
    }
    int cx = x + w/2 - dw/2;
    int cy = y + h/2 - dh/2;

    set_position(Gtk_FLAG(WIN_POS_NONE));
    set_uposition(cx, cy);
}


void DialogBox::center(const Gtk::Widget& widget)
{
    if (widget.is_mapped())
    {
/* #if defined (GTKMM_2)
        Glib::RefPtr<Gdk::Window> win =
            const_cast<Gtk::Widget&>(widget).get_window();

        gint x = 0, y = 0, w, h;
        win->get_origin(x, y);
        win->get_size(w, h);

        center(x, y, w, h);
#else
        Gdk_Window win = widget.get_window();

        gint x = 0, y = 0, w, h;
        win.get_origin(x, y);
        win.get_size(w, h);

#endif */

        gint x = 0, y = 0, w = 0, h = 0;

        Gtk_WINDOW(&widget)->get_origin(x, y);
        Gtk_WINDOW(const_cast<Widget*>(&widget))->get_size(w, h);

        center(x, y, w, h);
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
