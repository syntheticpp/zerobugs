//
// $Id: select_pid_dialog.cpp 720 2010-10-28 06:37:54Z root $
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
#include <boost/format.hpp>
#include "zdk/zero.h"
#include "gtkmm/button.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/connect.h"
#include "gtkmm/ctree.h"
#include "gtkmm/flags.h"
#include "gtkmm/frame.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/sorter.h"
#include "select_pid_dialog.h"
#include "text_entry.h"
#include "target/target.h"


using namespace std;


////////////////////////////////////////////////////////////////
SelectPidDialog::SelectPidDialog
(
    Debugger&       debugger,
    pid_t           exceptPid,
    const char*     title
)
  : DialogBox(btn_ok_cancel, title)
  , debugger_(debugger)
  , pid_(exceptPid)
  , tree_(NULL)
  , entry_(NULL)
{
    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow);
#ifdef GTKMM_2
    sw->set_shadow_type(Gtk_FLAG(SHADOW_IN));
#endif
    sw->set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));
    get_vbox()->pack_start(*sw);

    vector<const char*> titles;
    titles.push_back("PID");
    titles.push_back("Full Path");
    titles.push_back("Name");

    tree_ = manage(new Gtk::CTree(titles));
    sw->add(*tree_);

    tree_->column(0).set_width(140);
    tree_->column(1).set_width(340);

    Gtk_set_size(tree_, 590, 340);
#ifdef GTKMM_2
    set_resizable(true);

    tree_->get_column(0)->set_sort_column(0);
    tree_->get_column(1)->set_sort_column(1);
    tree_->get_column(2)->set_sort_column(2);
#else
    tree_->set_line_style(GTK_CTREE_LINES_DOTTED);
    tree_->column(0).set_width(150);
    tree_->column(0).set_passive();
    tree_->column(1).set_passive();
#endif
    set_numeric_sort(*tree_, 0, 10);

    Gtk::Button* btn = add_button("_Refresh");
    Gtk_CONNECT_0(btn, clicked, this, &SelectPidDialog::refresh);
    get_button_box()->set_layout(Gtk_FLAG(BUTTONBOX_END));

    if (debugger.options() & Debugger::OPT_ACCEPT_TARGET_PARAM)
    {
        Gtk::Frame* frame = manage(new Gtk::Frame("Target Parameters"));
        frame->set_border_width(3);

        RefPtr<Properties> prop = debugger.properties();
        entry_ = manage(new TextEntry(*this, prop, "target"));
        entry_->set_border_width(6);
        entry_->set_allow_empty(true);

        frame->add(*entry_);
        get_vbox()->pack_end(*frame, false, false, 10);

        if (RefPtr<Thread> thread = debugger.current_thread())
        {
            if (RefPtr<Target> target = thread->target())
            {
                entry_->set_text(target->id());
            }
        }
    }
    refresh();
}


////////////////////////////////////////////////////////////////
void SelectPidDialog::refresh()
{
    tree_->clear();
    if (entry_)
    {
        targetParam_ = entry_->get_text();
    }
    debugger_.enum_user_tasks(this, targetParam_.c_str());
}


////////////////////////////////////////////////////////////////
static bool insert_pid(
    const Gtk::CTree::RowList&  rows,
    const Gtk::CTree::Element&  elem,
    const string&               ppid,
    bool                        selectable)
{
    Gtk::CTree::RowList::const_iterator i = rows.begin();
    for (; i != rows.end(); ++i)
    {
        Gtk::CTree::Row row = Gtk::get_row(rows, *i);
        Gtk::CTree::Cell pidCell = row[0];

        if (insert_pid(row.subtree(), elem, ppid, selectable))
        {
            return true;
        }
        else if (pidCell.get_text() == ppid)
        {
            if (!row.get_selectable())
            {
                selectable = false;
            }
            row.subtree().push_back(elem);

            if (selectable)
            {
                row.expand();
            }
            else
            {
                row.subtree().back().set_selectable(false);
            }
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////
void SelectPidDialog::notify(const Runnable* task)
{
    assert(task);
    if (!task->name() || !*task->name())
    {
        return;
    }

    vector<string> sv;
    string name = basename(task->name());

    sv.push_back((boost::format("%1%") % task->pid()).str());
    sv.push_back(task->name());
    sv.push_back(name);

    const string ppid((boost::format("%1%") % task->ppid()).str());
    Gtk::CTree::RowList rows = tree_->rows();

    const bool selectable = (task->pid() != pid_);
    if (!insert_pid(rows, Gtk::CTree::Element(sv), ppid, selectable))
    {
        rows.push_back(Gtk::CTree::Element(sv));
    }
}

////////////////////////////////////////////////////////////////
pid_t SelectPidDialog::run(Gtk::Widget* centerOverWidget)
{
    pid_t result = 0;

    if (DialogBox::run(centerOverWidget) == btn_ok)
    {
        Gtk::CTree_Helpers::SelectionList& sel = tree_->selection();
        if (!sel.empty())
        {
            Gtk::CTree::Row row = Gtk::get_selected_row(sel);

            result = strtoul(row[0].get_text().c_str(), 0, 0);
            if (entry_)
            {
                targetParam_ = entry_->get_text();
            }
        }
    }
    return result;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
