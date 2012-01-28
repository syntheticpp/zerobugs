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
#include <assert.h>
#include "gtkmm/box.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/flags.h"
#include "gtkmm/resize.h"
#include "text_entry.h"
#include "text_entry_box.h"
#include "zdk/check_ptr.h"
#include "entry_dialog.h"


EntryDialog::EntryDialog
(
    const std::string&  message,
    Properties*         props,
    const char*         title,
    const char*         pixmap[]
)
  : MessageBox(message, btn_ok_cancel, title, pixmap)
  , entryBox_(manage(new TextEntryBox(*this, props, title)))
{
    assert(title);

    get_vbox()->add(*entryBox_);
    get_hbox()->set_spacing(0);
    get_vbox()->set_spacing(0);

    get_button_box()->set_layout(Gtk_FLAG(BUTTONBOX_END));
}


std::string EntryDialog::run(const Gtk::Widget* w)
{
    std::string result;

    CHKPTR(text_entry())->grab_focus();

    if (MessageBox::run(w) == btn_ok)
    {
        result = text_entry()->get_text();
    }
    return result;
}


Gtk::Entry* EntryDialog::get_entry()
{
    return CHKPTR(text_entry())->get_entry();
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
