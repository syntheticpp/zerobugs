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

#include "text_entry.h"
#include "text_entry_box.h"
#include "gtkmm/resize.h"


TextEntryBox::TextEntryBox
(
    DialogBox& dialog,
    WeakPtr<Properties> props,
    const char* label
)
  : entry_(manage(new TextEntry(dialog, props, label)))
{
    assert(label);
    Gtk_set_size(entry_, 320, -1);

    set_border_width(10);
    set_spacing(0);

    add(*entry_);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
