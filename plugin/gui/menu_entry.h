#ifndef MENU_ENTRY_H__4577C73D_A8F6_4DE9_9D2A_41A9A5A315B5
#define MENU_ENTRY_H__4577C73D_A8F6_4DE9_9D2A_41A9A5A315B5
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
#include "gtkmm/menu.h"

class MainWindow;


class MenuEntry
{
public:
    virtual ~MenuEntry() { }
    virtual void create(MainWindow*, Gtk::Menu_Helpers::MenuList&) const = 0;
};


class MenuItemEntry : public MenuEntry
{
public:
    MenuItemEntry(const char* name,
                  const char* accel,
                  unsigned long mask,
                  void (MainWindow::*func)(),
                  const char* path = NULL,
                  const Gtk::BuiltinStockID* = NULL);

    void create(MainWindow*, Gtk::Menu_Helpers::MenuList&) const;

private:
    const char* name_;
    const char* accel_;
    unsigned long mask_;
    void (MainWindow::*func_)();
    const char* path_;
    const Gtk::BuiltinStockID* stockID_;
};


class MenuSeparator : public MenuEntry
{
public:
    void create(MainWindow*, Gtk::Menu_Helpers::MenuList&) const;
};


#endif // MENU_ENTRY_H__4577C73D_A8F6_4DE9_9D2A_41A9A5A315B5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
