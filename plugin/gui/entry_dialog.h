#ifndef ENTRY_BOX_H__4D7D0391_1C85_4D5C_B5E1_6564CF8F5851
#define ENTRY_BOX_H__4D7D0391_1C85_4D5C_B5E1_6564CF8F5851
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
#include "message_box.h"
#include "text_entry_box.h"
#include "zdk/properties.h"


namespace Gtk
{
    class Box;
    class Entry;
}


/**
 * A dialog that contains one text entry field.
 * The text field as a drop down list associated with it
 * for showing a history.
 */
class ZDK_LOCAL EntryDialog : public MessageBox
{
public:
    EntryDialog(const std::string& message,
                Properties*        props,
                const char*        title,
                const char*        pixmap[] = 0);

    std::string run(const Gtk::Widget* = 0);

    TextEntry* text_entry()
    {
        return entryBox_->entry();
    }

    Gtk::Box* get_box() { return entryBox_; }

protected:
    Gtk::Entry* get_entry();

private:
    TextEntryBox* entryBox_;
};

#endif // ENTRY_BOX_H__4D7D0391_1C85_4D5C_B5E1_6564CF8F5851
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
