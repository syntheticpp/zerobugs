//
// $Id: edit_watchpoint_dlg.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <iomanip>
#include <sstream>
#include "gtkmm/button.h"
#include "edit_watchpoint_dlg.h"


static const char* msg =
    "Select the memory breakpoints (watchpoints)"
    " that you want to delete";


EditWatchPointDialog::EditWatchPointDialog
(
    ButtonID buttons,
    const char* title
)
: SelectDialog(buttons, title, msg)
{
    if (Gtk::Button* btn = get_ok_button())
    {
        btn->remove();
        btn->add_label("Delete", 0.5);
    }
}


Gtk::Widget& EditWatchPointDialog::add_action(
    addr_t addr,
    BreakPoint::Action* action)
{
    assert(action);

    std::ostringstream s;

    s << std::hex << std::setw(2 * sizeof(addr_t));
    s << std::setfill('0') << addr;
    s << ' ' << action->name();

    if (BreakPointAction::Info* info =
        interface_cast<BreakPointAction::Info*>(action))
    {
        s << " (" << info->description() << ')';
    }
    return add_item(s.str(), action);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
