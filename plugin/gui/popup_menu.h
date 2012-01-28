#ifndef POPUP_MENU_H__14444DA2_F1F8_4BFF_8E8F_B39CC0681FDD
#define POPUP_MENU_H__14444DA2_F1F8_4BFF_8E8F_B39CC0681FDD
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
#include "gtkmm/events.h"
#include "gtkmm/menu.h"

class ZDK_LOCAL PopupMenu : public Gtk::Menu
{
public:
    virtual ~PopupMenu()
    {
    }

    template<typename T> T* add_manage_item(T* item)
    {
        if (item)
        {
            manage(item)->show();
            append(*item);
        }
        return item;
    }

protected:
    virtual void on_selection_done()
    {
        Gtk::Menu::on_selection_done();
        delete this;
    }
};

#endif // POPUP_MENU_H__14444DA2_F1F8_4BFF_8E8F_B39CC0681FDD
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
