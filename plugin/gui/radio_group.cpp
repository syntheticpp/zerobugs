//
// $Id: radio_group.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cassert>
#include <iostream>
#include <memory>
#include <gdk/gdkkeysyms.h>
#include "gtkmm/accelgroup.h"
#include "gtkmm/box.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "gtkmm/radiobutton.h"
#include "gtkmm/widget.h"
#include "zdk/properties.h"
#include "dialog_box.h"
#include "radio_group.h"


using namespace std;
using namespace Gtk;
using namespace SigC;


RadioGroupHelper::RadioGroupHelper(Window& wnd, Properties* prop)
    : wnd_(wnd), prop_(prop)
{
    assert(prop_);
}


RadioGroupHelper::~RadioGroupHelper()
{
}


/* Creates a left-top-aligned check button, that toggles
   the boolean property of the given name. */

Widget& RadioGroupHelper::create_button(
    const string& name,
    const char* prop,
    RadioButtonGroup* group)
{
    /* temporarily wrap in auto_ptr, just in case
       something throws before we're done */

    auto_ptr<CheckButton> btn;
    if (group)
    {
        btn.reset(manage(new RadioButton(*group, name, .0)));
    }
    else
    {
        btn.reset(manage(new CheckButton(name, .0)));
    }
    map_.insert(make_pair(prop, btn.get()));

    if (prop == 0)
    {
        prop = name.c_str();
    }
    assert(prop_);
    if (prop_->get_word(prop, false))
    {
        btn->set_active(true);
        on_toggle_impl(prop, true);
    }
    if (btn->get_active())
    {
        prop_->set_word(prop, true);
    }
    Gtk_CONNECT_1(btn, toggled, this, &RadioGroupHelper::on_toggle, prop);

    if (Gtk_ACCEL_GROUP_PTR accelGroup = wnd_.get_accel_group())
    {
        int accel = 0;
        if (Label* label = dynamic_cast<Label*>(btn->get_child()))
        {
            accel = Gtk_get_mnemonic_keyval(label);
        }
        if (accel != GDK_VoidSymbol)
        {
        #if !GTKMM_2
            Gtk_ADD_ACCEL(*btn, "clicked",
                                 accelGroup,
                                 accel,
                                 Gdk_FLAG(MOD_NONE),
                                 Gtk_FLAG(ACCEL_LOCKED));
        #endif
            Gtk_ADD_ACCEL(*btn, "clicked",
                                 accelGroup,
                                 accel,
                                 Gdk_FLAG(MOD1_MASK),
                                 Gtk_FLAG(ACCEL_LOCKED));
        }
    }
    return *btn.release();
}


void RadioGroupHelper::on_toggle(const char* name)
{
    word_t val = prop_->get_word(CHKPTR(name), 0);
    val ^= 1;

    prop_->set_word(name, val);
    on_toggle_impl(name, val);
}


Gtk::ToggleButton*
RadioGroupHelper::get_button(const char* prop) const
{
    Gtk::ToggleButton* btn = 0;

    ButtonMap::const_iterator i = map_.find(prop);
    if (i != map_.end())
    {
        btn = i->second;
    }
    return btn;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
