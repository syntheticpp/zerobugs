#ifndef SELECT_PID_DIALOG_H__662DCD5D_7CD1_4C02_84BF_29DB001B7DB0
#define SELECT_PID_DIALOG_H__662DCD5D_7CD1_4C02_84BF_29DB001B7DB0
//
// $Id: select_pid_dialog.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/enum.h"
#include "dialog_box.h"

class Debugger;
class Runnable;

class TextEntry;
namespace Gtk
{
    class CTree;
}

/**
 * A dialogue for selecting a process to attach the debugger to.
 * Shows a list of tuples: (PID, Full Path, Name)
 */
class ZDK_LOCAL SelectPidDialog
    : public DialogBox
    , EnumCallback<const Runnable*>
{
public:
    explicit SelectPidDialog(
        Debugger&,
        pid_t exceptPid = 0,
        const char* title = 0);

    pid_t run(Gtk::Widget* = 0);

    const std::string& target_param() const { return targetParam_; }

private:
    void notify(const Runnable*);

    void refresh();

private:
    Debugger&       debugger_;
    pid_t           pid_;
    Gtk::CTree*     tree_;
    TextEntry*      entry_;
    std::string     targetParam_;
};

#endif // SELECT_PID_DIALOG_H__662DCD5D_7CD1_4C02_84BF_29DB001B7DB0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
