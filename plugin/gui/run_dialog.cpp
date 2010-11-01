//
// $Id: run_dialog.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cassert>
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "gtkmm/main.h"
#include "gtkmm/widget.h"
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/zero.h"
#include "debugger_environ.h"
#include "env_dialog.h"
#include "file_selection.h"
#include "slot_macros.h"
#include "run_dialog.h"

using namespace std;
using namespace SigC;


RunDialog::RunDialog
(
    Debugger&       dbg,
    const string&   text,
    const char*     title
)
    : EntryDialog(text, dbg.properties(), title)
    , debugger_(dbg)
    , chkBtn_(0)
    , fileSel_(0)
{
    Gtk::ButtonBox* box = manage(new Gtk::VButtonBox());
    get_hbox()->add(*box);
    get_hbox()->set_spacing(5);
    get_vbox()->set_spacing(0);

    // --- Browse button
    Gtk::Button* btn = manage(new Gtk::Button("Browse..."));
    box->add(*btn);
    box->set_layout(Gtk_FLAG(BUTTONBOX_END));
    Gtk_CONNECT_0(btn, clicked, this, &RunDialog::browse);

    chkBtn_ = manage(new Gtk::CheckButton(
        "Expand Command Line Arguments in Shell", .0));
    get_box()->add(*chkBtn_);

    // --- Environment button
    btn = manage(new Gtk::Button("_Environment..."));
    btn->set_flags(Gtk_FLAG(CAN_DEFAULT) | Gtk_FLAG(CAN_FOCUS));

    get_button_box()->add(*btn);
    Gtk_CONNECT_0(btn, clicked, this, &RunDialog::edit_environ);

    add_button_accelerator(*btn);

    get_label()->set_justify(Gtk_FLAG(JUSTIFY_LEFT));
}


BEGIN_SLOT(RunDialog::browse,())
{
    assert(fileSel_ == 0);

    FileSel fileSel("Select Program File", this->get_icon());
    fileSel.set_transient_for(*this);

    close_on_cancel(fileSel);

    Gtk_CONNECT_SLOT(&fileSel, destroy, Gtk::Main::quit);
    Gtk_CONNECT_0(fileSel.get_ok_button(), clicked, this, &RunDialog::file_selected);

    fileSel.hide_fileop_buttons();
    fileSel.show();
    fileSel.set_modal(true);

    assert(fileSel_ == 0);
    {
        Temporary<Gtk::FileSelection*> setInScope(fileSel_, &fileSel);
        Gtk::Main::run();
    }
    assert(fileSel_ == 0);
}
END_SLOT()


BEGIN_SLOT(RunDialog::file_selected,())
{
    assert(fileSel_);

    string path = fileSel_->get_filename();

    if (!path.empty())
    {
        if (path.find(" ") != string::npos)
        {
            path = '\"' + path + '\"';
        }
        CHKPTR(get_entry())->set_text(path);
    }
    Gtk_POPDOWN(fileSel_);
}
END_SLOT()


/**
 * Show a dialog for editing the environment for
 * the debugged program (that we are about to execute)
 */
BEGIN_SLOT(RunDialog::edit_environ, ())
{
    DebuggerEnvironment env(debugger_);

    EnvDialog dlg(env);
    dlg.set_transient_for(*this);
    dlg.run();
}
END_SLOT()


bool RunDialog::expand_args_in_shell() const
{
    return CHKPTR(chkBtn_)->get_active();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
