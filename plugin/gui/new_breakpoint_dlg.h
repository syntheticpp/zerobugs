#ifndef NEW_BREAKPOINT_DLG_H__0AB8E9C0_D1CE_48F9_9A32_E650609310B0
#define NEW_BREAKPOINT_DLG_H__0AB8E9C0_D1CE_48F9_9A32_E650609310B0
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include "dialog_box.h"
#include "file_selection.h"
#include "text_entry_box.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/notebook.h"

class Properties;

/**
 * A dialog for entering a new breakpoint.
 *
 * The breakpoint may be specified by:
 *  1) symbol name (mangled or demangled)
 *  2) address in hexadecimal
 *  3) source file and line number
 * Each of these options is implemented as a tab in a
 * notebook control.
 *
 */
class ZDK_LOCAL NewBreakPointDialog : public DialogBox
{
public:
    NewBreakPointDialog(const char*, Properties*);

    TextEntry::AutoCompleteSignal& auto_complete_signal();

    std::string run(Gtk::Widget*);

    SigC::Signal1<void, bool> use_unmapped_toggled;

private:
    void add_addr_page();

    void add_function_page();
    void on_use_unmapped_toggled();

    void add_source_page();

    void browse();
    void file_selected();

private:
    WeakPtr<Properties> props_;

    // we use a Notebook adapter rather than a plain
    // notebook, for backwards compatibility with Gtk 1.2
    Notebook_Adapt* book_;

    TextEntryBox* funcNameEntry_;
    TextEntryBox* addrEntry_;
    TextEntryBox* fileEntry_;

    Gtk::FileSelection* fileSel_;
    Gtk::CheckButton* lookupUnmapped_;
};

#endif // NEW_BREAKPOINT_DLG_H__0AB8E9C0_D1CE_48F9_9A32_E650609310B0
