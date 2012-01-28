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
#include "gtkmm/button.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/connect.h"
#include "gtkmm/clist.h"
#include "gtkmm/flags.h"
#include "gtkmm/entry.h"
#include "gtkmm/label.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/table.h"
#include "zdk/check_ptr.h"
#include "dharma/sarray.h"
#include "edit_in_place.h"
#include "message_box.h"
#include "slot_macros.h"
#include "icons/question.xpm"
#include "env_dialog.h"

using namespace std;



EnvDialog::EnvDialog(Environment& env)
    : DialogBox(btn_ok_cancel, "Environment")
    , env_(env)
    , list_(0)
{
    Gtk::Label* label = manage(new Gtk::Label(
        "\nDouble-click on value to edit "
        "variables, click OK to commit the changes", .0));

    get_vbox()->pack_start(*label, false, false);

    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow);
    get_vbox()->add(*sw);

    sw->set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_ALWAYS));

    static const char* titles[] = { "Name", "Value", 0 };
    list_ = manage(new EditableList(titles));

    sw->add(*list_);
    list_->set_column_editable(1, true);
    list_->column(0).set_width(200);
    list_->column(1).set_width(300);

    Gtk_CONNECT_0(list_, cell_edit, this, &EnvDialog::on_cell_edit);
    Gtk_CONNECT_0(list_, click_column, this, &EnvDialog::on_click_column);

    // "Add": allows user to add more environment variables
    Gtk::Button* btn = add_button("_Add");
    Gtk_CONNECT_1(btn, clicked, this, &EnvDialog::add_var, btn);

    // "Remove" button: for deletion of selected row
    btn = add_button("_Remove");
    Gtk_CONNECT_0(btn, clicked, this, &EnvDialog::remove);
    btn->set_sensitive(false);

#if defined (GTKMM_2)
    Gtk_CONNECT_1(list_, cursor_changed, this, &EnvDialog::on_selection_changed, btn);
#else
    // gray out the "Remove" button when nothing selected
    Gtk_CONNECT_1(list_, select_row, this, &EnvDialog::on_selection_changed, btn);
    Gtk_CONNECT_1(list_, unselect_row, this, &EnvDialog::on_selection_changed, btn);
#endif
    // "Reset" button
    btn = add_button("Reset to _Default");
    Gtk_CONNECT_1(btn, clicked, this, &EnvDialog::populate, true);

    Gtk_set_size(this, 660, 500);
    Gtk_set_resizable(this, true);

    populate(false);

    btn = get_ok_button();
    if (btn)
    {
        btn->grab_default();
        btn->grab_focus();
    }
}


void EnvDialog::populate(bool reset)
{
    list_->rows().clear();
    const char* const* envp = env_.get(reset);

    for (const char* const* var = envp; *var; ++var)
    {
        if (const char* p = strchr(*var, '='))
        {
            vector<string> items;

            items.push_back(string(*var, p - *var));
            items.push_back(++p);

            list_->rows().push_back(items);
        }
    }
}


void EnvDialog::on_button(ButtonID btn)
{
    if (btn == btn_ok)
    {
        // commit the changes to the environment
        vector<string> v;

        Gtk::CList::RowList rows = list_->rows();

        Gtk::CList::RowList::const_iterator i = rows.begin();
        for (; i != rows.end(); ++i)
        {
            Gtk::CList::Row row = Gtk::get_list_row(i, *list_);
            const string val = row[0].get_text()
                       + "=" + row[1].get_text();
            v.push_back(val);
        }
        SArray sarray(v.begin(), v.end());
        env_.set(sarray.cstrings());
    }
    DialogBox::on_button(btn);
}


#ifdef GTKMM_2
static void destroy(Gtk::Widget* w)
{
    w->hide();
    delete w;
}


static void enable_btn(Gtk::Widget* w)
{
    w->set_sensitive(true);
}
#endif // GTKMM_2


/**
 * Insert a dialog at the bottom of the list,
 * for entering a new environment variable
 */
BEGIN_SLOT(EnvDialog::add_var, (Gtk::Widget* w))
{
    get_ok_button()->set_sensitive(false);
    CHKPTR(w)->set_sensitive(false);

    Gtk::HBox* box = manage(new Gtk::HBox());
    get_vbox()->pack_start(*box, false, false);
    box->set_border_width(5);

    Gtk::Table* table = manage(new Gtk::Table(3, 2, false));
    box->pack_start(*table);

    Gtk::Label* label = manage(new Gtk::Label("Name:"));
    table->attach(*label, 0, 1, 0, 1,
        Gtk_FLAG(ATTACH_NONE), Gtk_FLAG(ATTACH_NONE));

    Gtk::Entry* name = manage(new Gtk::Entry());
    table->attach(*name, 1, 2, 0, 1,
        Gtk_FLAG(FILL) | Gtk_FLAG(EXPAND), Gtk_FLAG(ATTACH_NONE), 2, 2);

    label = manage(new Gtk::Label("Value:"));
    table->attach(*label, 0, 1, 1, 2,
        Gtk_FLAG(ATTACH_NONE), Gtk_FLAG(ATTACH_NONE));

    Gtk::Entry* value = manage(new Gtk::Entry());
    table->attach(*value, 1, 2, 1, 2,
        Gtk_FLAG(FILL) | Gtk_FLAG(EXPAND), Gtk_FLAG(ATTACH_NONE), 2, 2);

    // the Done button
    Gtk::Button* btn = manage(new Gtk::Button("Done"));
    table->attach(*btn, 2, 3, 0, 1,
        Gtk_FLAG(FILL) | Gtk_FLAG(EXPAND), Gtk_FLAG(ATTACH_NONE), 2, 2);

    btn->set_sensitive(false);
    Gtk_CONNECT_1(btn, clicked, w, &Gtk::Widget::set_sensitive, true);
    Gtk_CONNECT_2(btn, clicked, this, &EnvDialog::add_variable, name, value);
#if GTKMM_2
    btn->signal_clicked().connect(sigc::bind(sigc::ptr_fun(destroy), box));
#else
    Gtk_CONNECT(btn, clicked, box->destroy.slot());
#endif
    Gtk_CONNECT_2(name, changed, this, &EnvDialog::on_entry_changed, name, btn);

    // the Close button
    btn = manage(new Gtk::Button("Close"));

    table->attach(*btn, 2, 3, 1, 2,
        Gtk_FLAG(FILL) | Gtk_FLAG(EXPAND), Gtk_FLAG(ATTACH_NONE), 2, 2);

    Gtk_CONNECT_1(btn, clicked, w, &Gtk::Widget::set_sensitive, true);

#if GTKMM_2
    btn->signal_clicked().connect(sigc::bind(sigc::ptr_fun(destroy), box));
    box->signal_hide().connect(
        sigc::bind(sigc::ptr_fun(enable_btn), get_ok_button()));
#else
    btn->clicked.connect(box->destroy.slot());
    box->destroy.connect(bind(slot(get_ok_button(), &Widget::set_sensitive), true));
#endif
    box->show_all();
}
END_SLOT()


/**
 * Add the variable, if not already in list of environment vars
 */
void EnvDialog::add_variable(Gtk::Entry* name, Gtk::Entry* value)
{
    assert(name);
    assert(value);

    vector<string> items;

    items.push_back(name->get_text());
    items.push_back(value->get_text());

    Gtk::CList::RowList rows = list_->rows();
    Gtk::CList::RowList::const_iterator i = rows.begin();
    for (; i != rows.end(); ++i)
    {
        Gtk::CList::Row row = Gtk::get_list_row(i, *list_);
        if (row[0].get_text() == items.front())
        {
            string title = "Variable `" + items.front() + "' already exists. ";

            if (items[1].empty())
            {
                title += "Delete?";
            }
            else
            {
                title += "Replace?";
            }
            MessageBox dlg(title, btn_yes_no_cancel, "Enviroment", question_xpm);
            dlg.set_transient_for(*this);

            if (dlg.run() == btn_yes)
            {
                if (items[1].empty())
                {
                    list_->rows().erase(i);
                }
                else
                {
                    row[1].set_text(items[1]);
                }
            }
            return;
        }
    }
    list_->rows().push_back(items);
}


/**
 * Remove selected rows
 */
BEGIN_SLOT(EnvDialog::remove,())
{
    Gtk::CList::SelectionList sel = list_->selection();
    Gtk::CList::SelectionList::const_iterator i(sel.begin());
    Gtk::CList::SelectionList::const_iterator const end(sel.end());

    vector<int> rows; // indices of rows to be deleted
    rows.reserve(sel.size());

    for (; i != end; ++i)
    {
        rows.push_back(Gtk::get_row_num(*i));
    }

    vector<int>::const_iterator j(rows.begin());
    for (; j != rows.end(); ++j)
    {
        list_->remove_row(*j);
    }
}
END_SLOT()


/**
 * Allow the user to remove a variable only when
 * there is at least one row selected in the list
 */
#ifdef GTKMM_2
BEGIN_SLOT(EnvDialog::on_selection_changed,(Gtk::Widget* w))
#else
BEGIN_SLOT(EnvDialog::on_selection_changed,
(
    int,            // row, ignored
    int,            // column, ignored
    GdkEvent*,      // event, ignored
    Gtk::Widget* w
))
#endif // !GTKMM_2
{
    w->set_sensitive(!list_->selection().empty());
}
END_SLOT()


/**
 * If variable name not empty, then make the "Done" button sensitive
 */
BEGIN_SLOT(EnvDialog::on_entry_changed,
(
    Gtk::Entry* entry,
    Gtk::Widget* w
))
{
    w->set_sensitive(!entry->get_text().empty());
}
END_SLOT()


void EnvDialog::on_click_column(int ncol)
{
#ifndef GTKMM_2
    CHKPTR(list_)->set_sort_column(ncol);
    CHKPTR(list_)->sort();
#endif
}


bool EnvDialog::on_cell_edit( CELL_EDIT_PARAM )
{
    const int nrow = Gtk::get_row_num(path);
    bool result = true;

    if (s.empty())
    {
        result = false;
        list_->remove_row(nrow);
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
