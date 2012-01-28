#ifndef HISTORY_DLG_H__3FAF2C42_C832_4283_A586_4F5A13AEDD8A
#define HISTORY_DLG_H__3FAF2C42_C832_4283_A586_4F5A13AEDD8A
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
#include "custom_tooltip.h"
#include "dialog_box.h"
#include "slot_macros.h"
#include "zdk/ref_ptr.h"
#include "zdk/history.h"

namespace Gtk
{
    class Button;
    class CList;
}

/**
 * Displays a history of debugged programs.
 */
class ZDK_LOCAL HistoryDialog : public DialogBox
{
public:
    HistoryDialog(Debugger&,
                  const RefPtr<History>&,
                  const char* title,
                  const char* current);

    ButtonID run(const Gtk::Widget* centerOnParent = 0);

    SigC::Signal1<void, const std::string&> execute_program;

    SigC::Signal2<void, pid_t, const std::string&> attach_to_program;

    SigC::Signal1<void, const HistoryEntry*> execute;

private:
    void populate_list();

    // internal slots, connected to right-hand buttons
#ifdef GTKMM_2
    void on_selection_change();
#else
    void on_selection_change(int, int, GdkEvent*);
#endif
    void on_attach();

    void on_delete();

    void on_exec();

    void on_reset_all();

    void gray_out_all_buttons();

    event_result_t on_button_press_event(GdkEventButton*);

    void popup_menu(GdkEventButton&, HistoryEntry&);
    void edit_environment(HistoryEntry*);
    void edit_command_line(HistoryEntry*);

private:
    typedef ToolTipped<Gtk::CList> List;
    Debugger&               debugger_;
    List*                   list_;
    std::string             current_;
    RefPtr<History>         history_;
    RefPtr<HistoryEntry>    entry_;
    int                     index_; // index of current selection

    Gtk::Button*            btnExec_;
    Gtk::Button*            btnAttach_;
    Gtk::Button*            btnDelete_;
    Gtk::Button*            btnLoad_;
    Gtk::Button*            btnClearAll_;
};
#endif // HISTORY_DLG_H__3FAF2C42_C832_4283_A586_4F5A13AEDD8A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
