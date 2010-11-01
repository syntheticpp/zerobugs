#ifndef SET_WATCHPOINT_DLG_H__DE3C4157_AA85_4095_AF1E_96FC782560BA
#define SET_WATCHPOINT_DLG_H__DE3C4157_AA85_4095_AF1E_96FC782560BA
//
// $Id: set_watchpoint_dlg.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "dialog_box.h"
#include "radio_group.h"
#include "zdk/watchtype.h"


class DebugSymbol; // fully defined in zdk/debug.h"
class TextEntry;

namespace Gtk
{
    class Box;
}


CLASS SetWatchPointDialog : public DialogBox, public RadioGroupHelper
{
public:
    SetWatchPointDialog(Properties*, RefPtr<DebugSymbol>, addr_t = 0);

    ~SetWatchPointDialog();

    SigC::Signal2<void, WatchType, addr_t> set_memory_watch;

    SigC::Signal2<void, RelType, const std::string&> set_value_watch;

    ButtonID run(const Gtk::Widget* = 0);

private:
    void on_toggle_impl(const char*, bool);

    void set_rel(RelType rel) { rel_ = rel; }

    void add_variable_condition(
        Properties*,
        Gtk::Box&,
        Gtk::RadioButtonGroup&);

    void on_button(ButtonID);

    int on_focus_in(GdkEventFocus*);

private:
    TextEntry*              addrEntry_;
    TextEntry*              condEntry_;
    WatchType               type_;
    RelType                 rel_;
    RefPtr<DebugSymbol>     sym_;
    Gtk::RadioButtonGroup   grp_;
};

#endif // SET_WATCHPOINT_DLG_H__DE3C4157_AA85_4095_AF1E_96FC782560BA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
