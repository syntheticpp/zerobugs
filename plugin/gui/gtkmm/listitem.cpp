// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <iostream>
#include "list.h"
#include "zdk/check_ptr.h"


using namespace std;


Gtk::ListItem::~ListItem()
{
}


Gtk::ListItem::ListItem() : userData_(NULL), index_(-1)
{
}



Gtk::ListItem::ListItem(const std::string& value, void* data)
    : value_(value), userData_(data), index_(-1)
{
}


bool Gtk::ListItem::on_button_press_event(GdkEventButton* event)
{
    bool result = Widget::on_button_press_event(event);

    result |= buttonPress_(event);
    return result;
}


void Gtk_list_item_select(Gtk::List& list, const Gtk::TreeRow& row)
{
    list.select(row);
}


void Gtk_list_item_deselect(Gtk::List& list, const Gtk::TreeRow& row)
{
    list.deselect(row);
}


void Gtk_list_item_select(Gtk::List& list, Gtk::TreeIter& iter)
{
    list.select(*iter);
}


void Gtk_list_item_deselect(Gtk::List& list, Gtk::TreeIter& iter)
{
    list.deselect(*iter);
}

void Gtk_list_item_select(Gtk::List& list, Gtk::ListItem* item)
{
    list.select_item(CHKPTR(item)->index());
}

void Gtk_list_item_deselect(Gtk::List& list, Gtk::ListItem* item)
{
    list.deselect_item(CHKPTR(item)->index());
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
