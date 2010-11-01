// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//-*- tab-width: 4; indent-tabs-mode: nil;  -*-
//vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
//$Id: select_dialog.cpp 714 2010-10-17 10:03:52Z root $
//
#include <iostream>
#include <memory>
#include "gtkmm/button.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/box.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/frame.h"
#include "gtkmm/label.h"
#include "gtkmm/list.h"
#include "gtkmm/listitem.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "zdk/check_ptr.h"
#include "select_dialog.h"

using namespace std;


SelectDialog::SelectDialog
(
   ButtonID    buttons,
   const char* title,
   const char* message,
   bool        needSelection

)
 : DialogBox(buttons, title)
 , sw_(0)
 , list_(0)
 , selectBtn_(0)
 , deselectBtn_(0)
 , needSelection_(needSelection)
{
    get_vbox()->set_border_width(2);
    if (message)
    {
       Gtk::Box* hbox = manage(new Gtk::HBox);
       get_vbox()->pack_start(*hbox, false, false);

       Gtk::Frame* frame = manage(new Gtk::Frame);
       hbox->pack_start(*frame);
       hbox->set_border_width(3);

       Gtk::Label* label = manage(new Gtk::Label(message, .0));
       frame->add(*label);

       label->set_padding(3, 5);
       Gtk_set_size(label, 570, -1);
       label->set_justify(Gtk_FLAG(JUSTIFY_LEFT));
       label->set_line_wrap(true);
    }
    sw_ = manage(new Gtk::ScrolledWindow);
    get_vbox()->pack_start(*sw_);

    sw_->set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));
    Gtk_set_size(sw_, 580, 300);

    list_ = manage(new Gtk::List);
    Gtk_add_with_viewport(sw_, *list_);
    list_->set_selection_mode(Gtk_FLAG(SELECTION_MULTIPLE));

    add_select_button("Select _All", true);
    add_select_button("_Deselect All", false);

    Gtk_set_resizable(this, true);

    if (needSelection)
    {
       if (Gtk::Button* btn = get_ok_button())
       {
           btn->set_sensitive(false);
       }
    }
    selConn_ = Gtk_CONNECT_0(list_, selection_changed,
      this, &SelectDialog::on_selection_changed);

    get_vbox()->show_all();
}


SelectDialog::~SelectDialog()
{
}


void SelectDialog::close_dialog()
{
   selConn_.disconnect();
   DialogBox::close_dialog();
}


void SelectDialog::add_select_button(const char* label, bool sel)
{
   Gtk::Button* btn = DialogBox::add_button(label);

   Gtk_CONNECT_1(btn, clicked, this, &SelectDialog::on_select_all, sel);

   if (sel)
   {
       assert(!selectBtn_);
       selectBtn_ = btn;
   }
   else
   {
       assert(!deselectBtn_);

       deselectBtn_ = btn;
       btn->set_sensitive(false);
   }

}


Gtk::Widget& SelectDialog::add_item(const string& label, void* data)
{
   assert(list_);

   auto_ptr<Gtk::ListItem> item(manage(new Gtk::ListItem(label)));
   item->set_user_data(data);
   list_->add(*item);

   item.release();
   return *item;
}


void SelectDialog::on_select_all(bool select)
{
#ifdef GTKMM_2
    if (select)
    {
        get_list()->select_all();
    }
    else
    {
        get_list()->unselect_all();
    }
#else
    Gtk::List::ItemList& items = get_list()->items();

    Gtk::List::ItemList::iterator i = items.begin();
    for (; i != items.end(); ++i)
    {
        if (select)
        {
            Gtk_list_item_select(*get_list(), i);
        }
        else
        {
           Gtk_list_item_deselect(*get_list(), i);
        }
    }
#endif
}


void SelectDialog::on_selection_changed()
{
    const bool empty = list_->selection().empty();
    const bool all =
        (list_->selection().size() == list_->items().size());

    if (needSelection_)
    {
        if (Gtk::Button* btn = get_ok_button())
        {
            btn->set_sensitive(!empty);
        }
    }

    CHKPTR(deselectBtn_)->set_sensitive(!empty);
    CHKPTR(selectBtn_)->set_sensitive(!all);

    on_selection_changed_impl();
}


void SelectDialog::on_selection_changed_impl()
{
}


/**
 * Return the selected items
 */
vector<Gtk::SelectionItem> SelectDialog::run(const Gtk::Widget* w)
{
    vector<Gtk::SelectionItem> result;

    if (DialogBox::run(w) != btn_ok)
    {
       return result;
    }

    assert(list_);

    if (list_)
    {
        result = Gtk_get_selection_items(list_->selection());
    }
    return result;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
