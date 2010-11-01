//
// $Id: menu_entry.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <assert.h>
#include "gtkmm/accelmap.h"
#include "gtkmm/connect.h"
#include "gtkmm/menuitem.h"
#include "main_window.h"
#include "menu_entry.h"
#include "states.h"

using namespace SigC;


MenuItemEntry::MenuItemEntry
(
    const char* name,
    const char* accel,
    unsigned long mask,
    void (MainWindow::*func)(),
    const char* path,
    const Gtk::BuiltinStockID* stockID
)
: name_(name)
, accel_(accel)
, mask_(mask)
, func_(func)
, path_(path)
, stockID_(stockID)
{
}


void
MenuItemEntry::create(MainWindow* wnd, Gtk::Menu_Helpers::MenuList& list) const
{
    assert(name_);

    if (accel_ && strcmp(accel_, "check") == 0)
    {
        list.push_back(Gtk::Menu_Helpers::CheckMenuElem(name_));
    }
    else
    {
        using namespace Gtk;
        using namespace Gtk::Menu_Helpers;

        AccelKey ak(accel_ ? accel_ : "");

        if (path_)
        {
            MenuElem elem(*Gtk_image_menu_item(stockID_, name_));
            list.push_back(elem);

            Gtk_set_accel_path(elem, path_);
            Gtk::AccelMap::change_entry(path_, ak);
        }
        else if (stockID_)
        {
            MenuElem elem(*Gtk_image_menu_item(stockID_, name_, ak));
            list.push_back(elem);
        }
        else
        {
            MenuElem elem(name_, ak);
            list.push_back(elem);
        }
    }
    if (func_)
    {
        Gtk_CONNECT_0(&Gtk_MENU_ITEM(list.back()), activate, wnd, func_);
    }
    Gtk_MENU_ITEM(list.back()).set_data(STATE_MASK, (void*)mask_);
}


void
MenuSeparator::create(MainWindow*, Gtk::Menu_Helpers::MenuList& list) const
{
    list.push_back(Gtk::Menu_Helpers::SeparatorElem());
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
