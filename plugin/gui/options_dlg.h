#ifndef OPTIONS_DLG_H__6F0F8021_30F7_44E2_B675_EB5B4E656E53
#define OPTIONS_DLG_H__6F0F8021_30F7_44E2_B675_EB5B4E656E53
//
// $Id: options_dlg.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "dialog_box.h"

class Debugger;
class MainWindow;
class Options;

/**
 * A Dialog Box which contains the Options pages
 */
class ZDK_LOCAL OptionsDialog : public DialogBox
{
public:
    explicit OptionsDialog(Debugger&);

    void run(MainWindow&);

private:
    Options* options_;
};
#endif // OPTIONS_DLG_H__6F0F8021_30F7_44E2_B675_EB5B4E656E53
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
