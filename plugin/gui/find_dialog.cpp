//
// $Id: find_dialog.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <iostream>
#include "zdk/check_ptr.h"
#include "zdk/properties.h"
#include "gtkmm/button.h"
#include "gtkmm/box.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/entry.h"
#include "gtkmm/frame.h"
#include "gtkmm/radiobutton.h"
#include "gtkmm/resize.h"
#include "text_entry.h"
#include "find_dialog.h"

using namespace std;
using namespace Gtk;
using namespace SigC;


FindDialog::FindDialog(Properties* prop)
    : DialogBox(btn_ok_cancel, "Find")
    , RadioGroupHelper(*this, prop)
    , prop_(prop)
    , textEntry_(0)
{
    Frame* frame = manage(new Frame);
    get_vbox()->add(*frame);
    Gtk_set_size(frame, 400, -1);

    VBox* box = manage(new VBox);
    frame->add(*box);
    frame->set_border_width(5);

    textEntry_ = manage(new TextEntry(*this, prop_, "find"));
    box->pack_start(*textEntry_, false, false);
    box->set_border_width(10);

    textEntry_->set_text(properties()->get_string("search", ""));
    if (size_t length = textEntry_->get_text_length())
    {
        textEntry_->select_region(0, length);
    }

    Box* hbox = manage(new HBox);
    hbox->set_border_width(10);
    box->pack_start(*hbox, false, false);

    // Radio buttons
    VBox* bbox = manage(new VBox);
    hbox->pack_start(*bbox, false, false);
    bbox->set_border_width(10);

    bbox->pack_start(create_button("Search _Forward", "search_forward", &grp_));
    bbox->pack_start(create_button("Search _Backwards", "search_backward", &grp_));

    // Check buttons
    bbox = manage(new VBox);
    hbox->pack_end(*bbox, false, false);
    bbox->set_border_width(10);
    bbox->pack_start(create_button("Case Insensitive", "case_ins"), false, false);
    //todo
    //bbox->add(create_button("Match Whole Words", "match_words"));
    frame->show();
}


DialogBox::ButtonID FindDialog::run(const Widget* w)
{
    CHKPTR(textEntry_)->grab_focus();
    ButtonID btnID = DialogBox::run(w);

    if (btnID != btn_ok)
    {
        return btnID;
    }
    string what = textEntry_->get_text();
    find_in_file(what);

    return btnID;
}


void FindDialog::set_text(const string& text)
{
    if (textEntry_)
    {
        textEntry_->set_text(text);
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
