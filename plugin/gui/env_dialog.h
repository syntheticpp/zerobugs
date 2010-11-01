#ifndef ENV_DIALOG_H__5E5A924C_8E4B_4EF1_A8AE_8A66D30A786F
#define ENV_DIALOG_H__5E5A924C_8E4B_4EF1_A8AE_8A66D30A786F
//
// $Id: env_dialog.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "dialog_box.h"
#include "edit_in_place.h"
#include "environment.h"


namespace Gtk
{
    class CList;
    class Entry;
    class TreePath;
}

// template<typename T> class EditableInPlace;


/**
 * A dialog for editing the environment for
 * programs executed under the debugger
 */
class EnvDialog : public DialogBox
{
    typedef EditableInPlace<Gtk::CList> EditableList;

public:
    explicit EnvDialog(Environment&);

private:
    virtual void on_button(ButtonID);

    void populate(bool resetEnvironment);

    /* show a dialog for adding a new variable */
    void add_var(Gtk::Widget*);

    /* actually add the newly entered variable */
    void add_variable(Gtk::Entry*, Gtk::Entry*);

    void remove();

    void on_entry_changed(Gtk::Entry*, Gtk::Widget*);

    void on_click_column(int);
#if GTKMM_2
    void on_selection_changed(Gtk::Widget*);
#else
    void on_selection_changed(int, int, GdkEvent*, Gtk::Widget*);
#endif
    bool on_cell_edit ( CELL_EDIT_PARAM );

private:
    Environment& env_;
    EditableInPlace<Gtk::CList>* list_;
};

#endif // ENV_DIALOG_H__5E5A924C_8E4B_4EF1_A8AE_8A66D30A786F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
