#ifndef MESSAGE_BOX_H__A2D4F116_8CCD_4535_8772_FD9B24BB2B2D
#define MESSAGE_BOX_H__A2D4F116_8CCD_4535_8772_FD9B24BB2B2D
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


/**
 * @note this was written back in the Gtk-- 1.2 days
 * a newer implementation might use MessageDialog
 */
class MessageBox : public DialogBox
{
public:
    MessageBox( const std::string& message,
                ButtonID,
                const char* title = 0,
                const char* pixmap[] = 0);
protected:
    Gtk::Box* get_hbox() const { return hbox_; }

    Gtk::Label* get_label() const { return label_; }

private:
    Gtk::Box*   hbox_;
    Gtk::Label* label_;
};

#endif // MESSAGE_BOX_H__A2D4F116_8CCD_4535_8772_FD9B24BB2B2D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
