#ifndef SIGNALS_DIALOG_H__8C256D3D_6B49_4B67_8AFD_D6E9CAE72919
#define SIGNALS_DIALOG_H__8C256D3D_6B49_4B67_8AFD_D6E9CAE72919
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
#include <vector>
#include "dialog_box.h"

class Debugger;

namespace Gtk
{
    class CheckButton;
}

/**
 * A dialog that lets the user specify how various
 * signals sent to the debuggee are handled. Options
 * are "pass" (debugger passes the signal to debuggee
 * or not) and "stop" (debugger stops so that the user
 * can enter a command, examine the debuggee, etc.
 * when signal is received).
 */
class SignalsDialog : public DialogBox
{
public:
    explicit SignalsDialog(Debugger&);

    void run(Gtk::Widget*);

private:
    typedef std::vector<Gtk::CheckButton*> ButtonList;

    Debugger& debugger_;

    ButtonList pass_;
    ButtonList stop_;
};

#endif // SIGNALS_DIALOG_H__8C256D3D_6B49_4B67_8AFD_D6E9CAE72919
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
