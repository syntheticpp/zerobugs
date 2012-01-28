#ifndef FIND_DIALOG_H__B436EF42_FE5B_4D49_AD6A_0A827BC9BC36
#define FIND_DIALOG_H__B436EF42_FE5B_4D49_AD6A_0A827BC9BC36
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
#include <string>
#include "dialog_box.h"
#include "radio_group.h"

struct Properties;  // zdk/properties.h
class TextEntry;


class ZDK_LOCAL FindDialog : public DialogBox, public RadioGroupHelper
{
public:
    explicit FindDialog(Properties*);

    /* These signals are emitted when clicking OK
       or hitting <Enter>. The search string is given
       to the slot, the other params are passed in via
       the Properties. */
    SigC::Signal1<void, const std::string&> find_in_file;

    SigC::Signal0<std::string> get_selection;

    ButtonID run(const Gtk::Widget* w = 0);

    void set_text(const std::string&);

private:
    Properties*     prop_;
    TextEntry*      textEntry_;
    Gtk::RadioButtonGroup grp_;
};

#endif // FIND_DIALOG_H__B436EF42_FE5B_4D49_AD6A_0A827BC9BC36
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
