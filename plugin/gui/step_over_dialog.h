#ifndef STEP_OVER_DIALOG_H__DD39ED11_63C5_4DF6_9589_48BA8FD41F53
#define STEP_OVER_DIALOG_H__DD39ED11_63C5_4DF6_9589_48BA8FD41F53
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: step_over_dialog.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "zdk/enum.h"
#include "zdk/shared_string.h"
#include "dialog_box.h"
#include "gtkmm/ctree.h"
#include "gtkmm/notebook.h"


class StepOverPage; // opaque


/**
 * Manage directories, files and functions that the
 * user wants to always step over.
 */
CLASS StepOverDialog : public DialogBox
                     , EnumCallback2<SharedString*, long>
{
    typedef SigC::Signal1<
        size_t, EnumCallback2<SharedString*, long>* > SignalPopulate;
    typedef SigC::Signal2<void, RefPtr<SharedString>, long> SignalModify;
    typedef std::vector<std::pair<RefPtr<SharedString>, long> > StepOverList;
    typedef SigC::Signal0<void> SignalClearAll;

    SignalPopulate              populate_;
    SignalModify                add_;
    SignalModify                delete_;
    Notebook_Adapt*             nbook_;
    std::vector<StepOverPage*>  pages_;
    Gtk::Button*                btnAdd_;
    Gtk::Button*                btnDelete_;
    StepOverList                toDelete_;
    SigC::Connection            connSwitchPage_;

private:
    void notify(SharedString*, long);
    void add_page(StepOverPage*);

    RefPtr<SharedString> selected_file(long* line = 0, bool remove = false);

    void step_over_add();
    void step_over_delete();

    void selection_changed();
    void on_selection_changed(Gtk::CTree::Row, int nColumn);
    void on_switch_page(GtkNotebookPage*, guint);

public:
    StepOverDialog();
    ~StepOverDialog();

    SignalPopulate& signal_populate() { return populate_; }
    SignalModify& signal_add() { return add_; }
    SignalModify& signal_delete() { return delete_; }

    size_t populate() { return populate_(this); }

    void run(const Gtk::Widget&);
};

#endif // STEP_OVER_DIALOG_H__DD39ED11_63C5_4DF6_9589_48BA8FD41F53
