// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <memory>
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/text.h"
#include "ensure_font.h"
#include "fixed_font.h"
#include "slot_macros.h"
#include "text_dialog.h"

#if GTKMM_2

bool TextDialog::on_map_event(GdkEventAny* event)
{
    if (text_)
    {
        ensure_monospace(font_, *text_);
    }
    return DialogBox::on_map_event(event);
}
#endif

using namespace std;


TextDialog::TextDialog(const char* title, size_t w, size_t h)
    : DialogBox(btn_close, title)
    , font_(fixed_font())
    , text_(manage(new Gtk::Text))
{
    auto_ptr<Gtk::Text> tmp(text_);
    ensure_monospace(font_, *text_);

    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow);
    get_vbox()->pack_start(*sw);
    sw->set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));
    sw->add(*tmp.release());

    Gtk_set_size(text_, w, h);
    Gtk_set_resizable(this, true);

    CHKPTR(text_)->set_editable(false);
    CHKPTR(text_)->set_line_wrap(false);
}


TextDialog::~TextDialog()
{
}


void TextDialog::insert_text(const string& text)
{
    text_->insert(font_, text);
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
