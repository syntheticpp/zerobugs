#ifndef RUN_DIALOG_H__9E27F34E_8A58_4D3C_8354_DBA105568F0B
#define RUN_DIALOG_H__9E27F34E_8A58_4D3C_8354_DBA105568F0B
//
// $Id: run_dialog.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "entry_dialog.h"

class Debugger;

namespace Gtk
{
    class CheckButton;
    class FileSelection;
}


class RunDialog : public EntryDialog
{
public:
    RunDialog(Debugger&, const std::string&, const char* title);

    bool expand_args_in_shell() const;

private:
    void browse();
    void file_selected();
    void edit_environ();

    Debugger&           debugger_;
    Gtk::CheckButton*   chkBtn_;
    Gtk::FileSelection* fileSel_;
};

#endif // RUN_DIALOG_H__9E27F34E_8A58_4D3C_8354_DBA105568F0B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
