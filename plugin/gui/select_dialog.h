#ifndef SELECT_DIALOG_H__CE947709_ED25_461E_9D3B_02360011C97D
#define SELECT_DIALOG_H__CE947709_ED25_461E_9D3B_02360011C97D
//
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
#include <cassert>
#include <vector>
#include "dialog_box.h"
#include "gtkmm/selectionitem.h"

namespace Gtk
{
    class Button;
    class List;
    class ScrolledWindow;
}


/**
 * A dialog for selecting generic items. Allows multiple
 * selection. Uses a list with CheckListItem elements.
 */
class SelectDialog : public DialogBox
{
public:
    SelectDialog(
        ButtonID buttons,
        const char* title,
        const char* message, // optional
        bool needSelection = true);

    virtual ~SelectDialog();

    virtual Gtk::Widget& add_item(const std::string& label, void* data);

    /// @return the selected items
    std::vector<Gtk::SelectionItem> run(const Gtk::Widget* = 0);

protected:
    void on_select_all(bool selected);

    void on_selection_changed();

    virtual void on_selection_changed_impl();

    Gtk::List* get_list() { assert(list_); return list_; }

    Gtk::ScrolledWindow* get_scrolled_window() { return sw_; }

    void close_dialog();

private:
    void add_select_button(const char* label, bool select);

    Gtk::ScrolledWindow* sw_;

    Gtk::List* list_;

    Gtk::Button* selectBtn_;

    Gtk::Button* deselectBtn_;

    bool needSelection_; /* OK button needs a non-empty selection? */

    SigC::Connection selConn_;
};

#endif // SELECT_DIALOG_H__CE947709_ED25_461E_9D3B_02360011C97D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
