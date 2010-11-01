#ifndef EDIT_WATCHPOINT_DLG_H__0B155C2C_326F_4379_B1BF_161E8958390C
#define EDIT_WATCHPOINT_DLG_H__0B155C2C_326F_4379_B1BF_161E8958390C
//
// $Id: edit_watchpoint_dlg.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zero.h"
#include "select_dialog.h"

/* Dialog for editing watchpoints (memory breakpoints) */
class EditWatchPointDialog : public SelectDialog
{
public:
    EditWatchPointDialog(ButtonID, const char*);

    /**
     * Add a list item that describes a breakpoint action.
     */
    Gtk::Widget& add_action(addr_t, BreakPoint::Action*);
};

#endif // EDIT_WATCHPOINT_DLG_H__0B155C2C_326F_4379_B1BF_161E8958390C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
