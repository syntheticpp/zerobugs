#ifndef TEXT_DIALOG_H__1D846105_FB3F_451C_B6AE_90F4A96DDE8B
#define TEXT_DIALOG_H__1D846105_FB3F_451C_B6AE_90F4A96DDE8B
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

#include "gtkmm/font.h"
#include "gtkmm/textfwd.h"
#include "dialog_box.h"


class TextDialog : public DialogBox
{
public:
    TextDialog(const char* title, size_t w, size_t h);

    virtual ~TextDialog();

    void insert_text(const std::string&);

    bool on_map_event(GdkEventAny* event);

private:
    Gdk_Font    font_;
    Gtk::Text*  text_;
};
#endif // TEXT_DIALOG_H__1D846105_FB3F_451C_B6AE_90F4A96DDE8B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
