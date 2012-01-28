// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include <string>
#include <vector>
#include "step_over_dialog.h"
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/connect.h"
#include "gtkmm/ctree.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "zdk/shared_string_impl.h"


using namespace Gtk;
using namespace std;

static const size_t PATH_WIDTH = 420;


/**
 * A page in the StepOverDialog
 */
CLASS StepOverPage : public HBox
{
    CTree* tree_; // use a tree rather than a list; one day
                  // I may want to display things hierarchically

protected:
    mutable vector<string>  titles_;// column titles

protected:
    CTree& tree() { assert(tree_); return *tree_; }

    virtual const vector<string>& column_titles() const
    {
        if (titles_.empty())
        {
            titles_.push_back("Path");
        }
        return titles_;
    }

    void add_tree()
    {
        ScrolledWindow* sw = manage(new ScrolledWindow);
        //sw->set_shadow_type(Gtk_FLAG(SHADOW_OUT));
        sw->set_policy(Gtk_FLAG(POLICY_NEVER), Gtk_FLAG(POLICY_AUTOMATIC));
        add(*sw);
        tree_ = manage(new CTree(column_titles()));
        Gtk_set_size(tree_, 500, 200);
        sw->add(*tree_);
    }

    virtual long selected_line(const CTree::Row& row) const = 0;

public:
    StepOverPage() : tree_(0) { }
    virtual const char* name() const = 0;
    virtual bool add_file(const char* fileName, long lineNum) = 0;

    RefPtr<SharedString> selected_file(long* line, bool remove)
    {
        RefPtr<SharedString> result;
        Gtk::CTree_Helpers::SelectionList& sel = tree_->selection();
        if (!sel.empty())
        {
            Gtk::CTree::Row row = Gtk::get_selected_row(sel);
            result = shared_string(row[0].get_text());
            if (line)
            {
                *line = selected_line(row);
            }
            if (remove)
            {
                tree_->rows().erase(row);
            }
        }
        return result;
    }

    CTree::SignalSelectionChanged signal_tree_select_row()
    {
        return tree_->signal_tree_select_row();
    }
};



CLASS StepOverAll : public StepOverPage
{
    virtual const char* name() const { return "All"; }

    virtual const vector<string>& column_titles() const
    {
        if (titles_.empty())
        {
            titles_.push_back("Path");
            titles_.push_back("Line");
        }
        return titles_;
    }
    virtual long selected_line(const CTree::Row& row) const
    {
        //return strtoul(row[1].get_text().c_str(), 0, 0);
        long line = reinterpret_cast<long>(row.get_data());
        return line;
    }

public:
    StepOverAll()
    {
        add_tree();
        tree().column(0).set_width(PATH_WIDTH);
    }
    virtual bool add_file(const char* fileName, long lineNum)
    {
        vector<string> columns;
        columns.push_back(fileName);
        ostringstream line;
        if (lineNum > 0)
        {
            line << lineNum;
        }
        else
        {
            line << "*";
        }
        columns.push_back(line.str());
        tree().rows().push_back(CTree::Element(columns));
        tree().rows().back().set_data(reinterpret_cast<void*>(lineNum));
        return true;
    }
};



CLASS StepOverDirs : public StepOverPage
{
    const char* name() const { return "Directories"; }

public:
    StepOverDirs()
    {
        add_tree();
    }
    virtual bool add_file(const char* fileName, long lineNum)
    {
        if (lineNum == -1)
        {
            vector<string> columns;
            columns.push_back(fileName);

            tree().rows().push_back(CTree::Element(columns));
            return true;
        }
        return false;
    }
    virtual long selected_line(const CTree::Row&) const
    {
        return -1;
    }
};


CLASS StepOverFiles : public StepOverPage
{
public:
    StepOverFiles()
    {
        add_tree();
    }
    virtual const char* name() const { return "Files"; }

    virtual bool add_file(const char* fileName, long lineNum)
    {
        if (lineNum == 0)
        {
            vector<string> columns;
            columns.push_back(fileName);

            tree().rows().push_back(CTree::Element(columns));
            return true;
        }
        return false;
    }
    virtual long selected_line(const CTree::Row&) const
    {
        return 0;
    }
};


CLASS StepOverFunctions : public StepOverPage
{
    virtual const vector<string>& column_titles() const
    {
        if (titles_.empty())
        {
            titles_.push_back("File");
            titles_.push_back("Line");
        }
        return titles_;
    }

public:
    StepOverFunctions()
    {
        add_tree();
        tree().column(0).set_width(PATH_WIDTH);
    }
    const char* name() const { return "Functions"; }
    virtual bool add_file(const char* fileName, long lineNum)
    {
        if (lineNum > 0)
        {
            vector<string> columns;
            ostringstream line;
            line << lineNum;
            columns.push_back(fileName);
            columns.push_back(line.str());

            tree().rows().push_back(CTree::Element(columns));
            return true;
        }
        return false;
    }
    virtual long selected_line(const CTree::Row& row) const
    {
        return strtoul(row[1].get_text().c_str(), 0, 0);
    }
};


static const char labelText[] =
    "Manage code locations that you want to always step over";

////////////////////////////////////////////////////////////////
//
// StepOverDialog implementation
//
StepOverDialog::StepOverDialog()
    : DialogBox(btn_ok_cancel, "Step Over")
    , nbook_(manage(new Notebook_Adapt))
    //, btnAdd_(manage(new Button("Add...")))
    , btnDelete_(manage(new Button("Delete")))
{
    //Gtk_CONNECT_0(btnAdd_, clicked, this, &StepOverDialog::step_over_add);
    Gtk_CONNECT_0(btnDelete_, clicked, this, &StepOverDialog::step_over_delete);
    // start with the delete buttons grayed out and activate them
    // when the user selects an entry
    btnDelete_->set_sensitive(false);
    Label* label = manage(new Label(labelText, .025, .5));
    HBox* hbox = manage(new HBox);
    get_vbox()->pack_start(*label, false, false, 12);
    get_vbox()->add(*hbox);

    hbox->add(*nbook_);
    ButtonBox* bbox = manage(new VButtonBox(Gtk_FLAG(BUTTONBOX_START)));
    hbox->pack_end(*bbox, false, false, 10);
    //bbox->pack_start(*btnAdd_, false, false);
    bbox->pack_start(*btnDelete_, false, false);

    add_page(new StepOverAll);
    add_page(new StepOverDirs);
    add_page(new StepOverFiles);
    add_page(new StepOverFunctions);
    Gtk_set_resizable(this, true);

    //re-evaluate buttons sensitivity when the current notebook page changes
    connSwitchPage_ =
        Gtk_CONNECT_AFTER_0(nbook_, switch_page, this, &StepOverDialog::on_switch_page);
    show_all();
}


StepOverDialog::~StepOverDialog()
{
    connSwitchPage_.disconnect();
}


void StepOverDialog::add_page(StepOverPage* page)
{
    manage(page);
    nbook_->append_page(*page, page->name());
    pages_.push_back(page);
    page->signal_tree_select_row().connect(
        Gtk_SLOT(this, &StepOverDialog::on_selection_changed));
}


void StepOverDialog::notify(SharedString* fileName, long lineNumber)
{
    if (!fileName)
    {
        return;
    }
    for (vector<StepOverPage*>::iterator i = pages_.begin(); i != pages_.end(); ++i)
    {
        (*i)->add_file(fileName->c_str(), lineNumber);
    }
}


RefPtr<SharedString> StepOverDialog::selected_file(long* line, bool remove)
{
    const int n = nbook_->get_current_page();

    if (n >= 0 && static_cast<size_t>(n) < pages_.size())
    {
        return pages_[n]->selected_file(line, remove);
    }
    return NULL;
}


/* todo

void StepOverDialog::step_over_add()
{
    long line = 0;
    if (RefPtr<SharedString> file = selected_file(line))
    {
        add_(file, line);
    }
}
*/


void StepOverDialog::step_over_delete()
{
    long line = 0;
    if (RefPtr<SharedString> file = selected_file(&line, true))
    {
        toDelete_.push_back(make_pair(file, line));
        selection_changed();
    }
}


void StepOverDialog::run(const Widget& w)
{
    assert(toDelete_.empty());

    if (DialogBox::run(&w, true) == btn_ok)
    {
        for (StepOverList::iterator i = toDelete_.begin(); i != toDelete_.end();)
        {
            delete_(i->first, i->second);
            i = toDelete_.erase(i);
        }
    }
}


void StepOverDialog::selection_changed()
{
    btnDelete_->set_sensitive(selected_file());
}


/**
 * Re-evaluate button(s) sensitivity when the current notebook
 * page changes.
 */
void StepOverDialog::on_switch_page(GtkNotebookPage*, guint)
{
    selection_changed();
}


void StepOverDialog::on_selection_changed(Gtk::CTree::Row, int)
{
    selection_changed();
}

