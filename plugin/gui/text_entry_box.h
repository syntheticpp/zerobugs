#ifndef TEXT_ENTRY_BOX_H__135B4E7B_4545_455D_B5EB_708B3894840B
#define TEXT_ENTRY_BOX_H__135B4E7B_4545_455D_B5EB_708B3894840B
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
#include "gtkmm/box.h"
#include "text_entry.h"

class DialogBox;
class Properties;


class ZDK_LOCAL TextEntryBox : public Gtk::VBox
{
    TextEntry* entry_;

public:
    TextEntryBox(DialogBox&, WeakPtr<Properties>, const char* labelText);

    TextEntry* entry() { return entry_; }

    TextEntry::AutoCompleteSignal& auto_complete_signal()
    {
        return entry_->auto_complete;
    }
};

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
#endif // TEXT_ENTRY_BOX_H__135B4E7B_4545_455D_B5EB_708B3894840B
