#ifndef MENU_CLICK_H__971B67C6_8D5A_46CA_83AA_7CB6F066622E
#define MENU_CLICK_H__971B67C6_8D5A_46CA_83AA_7CB6F066622E
//
// $Id: menu_click.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "zdk/zero.h"
#include "gtkmm/menu.h"



struct ZDK_LOCAL MenuClickContext
{
    virtual Thread* thread() const = 0;

    // virtual size_t line() const = 0;

    virtual size_t position() const = 0;
};


template<typename T>
struct ZDK_LOCAL MenuAction : public ZObject
{
    virtual ~MenuAction() { }

    virtual bool add_to(T*, Gtk::Menu&, MenuClickContext&) = 0;
};



#endif // MENU_CLICK_H__971B67C6_8D5A_46CA_83AA_7CB6F066622E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
