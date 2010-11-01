// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: new_breakpoint_dlg.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "generic/temporary.h"
#include "gtkmm/connect.h"
#include "gtkmm/label.h"
#include "gtkmm/notebook.h"
#include "gtkmm/resize.h"
#include "gtkmm/widget.h"
#include "zdk/check_ptr.h"
#include "zdk/properties.h"
#include "new_breakpoint_dlg.h"
#include "slot_macros.h"

using namespace std;
using namespace Gtk;


NewBreakPointDialog::NewBreakPointDialog
(
    const char* title,
    Properties* props
)
: DialogBox(btn_ok_cancel, title)
, props_(props)
, book_(manage(new Notebook_Adapt))
, funcNameEntry_(NULL)
, addrEntry_(NULL)
, fileEntry_(NULL)
, fileSel_(NULL)
, lookupUnmapped_(NULL)
{
    get_vbox()->add(*book_);

    add_function_page();
    add_addr_page();
    add_source_page();
}


/**
 * Add a Tab for the user to insert a new breakpoint by specifying
 * a function name (or fully-qualified method name).
 */
void NewBreakPointDialog::add_function_page()
{
    assert (!funcNameEntry_);

    funcNameEntry_ = manage(new TextEntryBox(*this, props_, "bpnt_func"));
    Gtk_set_size(funcNameEntry_, -1, 80);
    book_->add(*funcNameEntry_);

    assert(!lookupUnmapped_);

    // When checked, the autocomplete handler will also
    // look inside shared libraries that are not mapped
    // into memory yet.
    static const char btnLabel[] = "Auto-complete _unmapped symbols";
    lookupUnmapped_ = manage(new CheckButton(btnLabel));

    // TextEntryBox is a VBox
    funcNameEntry_->add(*lookupUnmapped_);

    Gtk_CONNECT(lookupUnmapped_, toggled,
                Gtk_SLOT(this, &NewBreakPointDialog::on_use_unmapped_toggled));

    add_button_accelerator(*lookupUnmapped_);

    book_->set_tab_label_text(*funcNameEntry_, "Function Name");
}


/**
 * Add a Tab for the user to insert a new breakpoint by address
 */
void NewBreakPointDialog::add_addr_page()
{
    addrEntry_ = manage(new TextEntryBox(*this, props_, "bpnt_addr"));
    book_->add(*addrEntry_);
    book_->set_tab_label_text(*addrEntry_, "Address");
}


/**
 * Add a Tab for the user to insert a new breakpoint by source file
 * and line number.
 */
void NewBreakPointDialog::add_source_page()
{
    Box* vbox = manage(new VBox);
    Box* hbox = manage(new HBox);

    vbox->pack_start(*hbox, false, false, 10);
    fileEntry_ = manage(new TextEntryBox(*this, props_, "bpnt_file"));

    Button* btn = manage(new Button("Browse..."));
    Gtk_CONNECT_0(btn, clicked, this, &NewBreakPointDialog::browse);

    Box* vbox2 = manage(new VBox);
    vbox2->pack_start(*btn, false, false, 10);

    hbox->pack_start(*vbox2, false, false, 3);
    hbox->add(*fileEntry_);

    book_->add(*vbox);
    book_->set_tab_label_text(*vbox, "Source Location");
}


BEGIN_SLOT(NewBreakPointDialog::on_use_unmapped_toggled,())
{
    use_unmapped_toggled.emit(lookupUnmapped_->get_active());
}
END_SLOT()


/**
 * relay the auto-complete signal (for the function name page / tab)
 */
TextEntry::AutoCompleteSignal& NewBreakPointDialog::auto_complete_signal()
{
    return funcNameEntry_->auto_complete_signal();
}


BEGIN_SLOT(NewBreakPointDialog::browse,())
{
    assert(fileSel_ == 0);

    FileSel fileSel("Select Source File", this->get_icon());
    fileSel.set_transient_for(*this);

    close_on_cancel(fileSel);

    Gtk_CONNECT_SLOT(&fileSel, destroy, Gtk::Main::quit);
    Gtk_CONNECT_0(fileSel.get_ok_button(), clicked, this,
                  &NewBreakPointDialog::file_selected);

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


BEGIN_SLOT(NewBreakPointDialog::file_selected,())
{
    assert(fileSel_);

    string path = fileSel_->get_filename();

    if (!path.empty())
    {
       /*
        if (path.find(" ") != string::npos)
        {
            path = '\"' + path + '\"';
        } */
        CHKPTR(fileEntry_)->entry()->set_text(path);
    }
    Gtk_POPDOWN(fileSel_);
}
END_SLOT()


string NewBreakPointDialog::run(Gtk::Widget* w)
{
    string result;

    book_->set_current_page(0);
    funcNameEntry_->grab_focus();

    if (DialogBox::run(w) == btn_ok)
    {
        switch (book_->get_current_page())
        {
        case 0:
            result = funcNameEntry_->entry()->get_text();
            break;

        case 1:
            result = addrEntry_->entry()->get_text();
            break;
        }
    }
    return result;
}


