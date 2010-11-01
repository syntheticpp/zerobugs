#ifndef MENU_ACTION_IMPL_H__F6FDEA83_4BBF_4628_B006_3579249427D1
#define MENU_ACTION_IMPL_H__F6FDEA83_4BBF_4628_B006_3579249427D1
//
// $Id: menu_action_impl.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zobject_impl.h"
#include "gtkmm/base.h"
#include "gtkmm/connect.h"
#include "gtkmm/menuitem.h"
#include "menu_click.h"


extern "C" void handle_ui_exception() throw();


template<typename T, bool needLiveThread = false, typename R = void>
class ZDK_LOCAL BaseMenuActionImpl
    : public ZObjectImpl<MenuAction<T> >
    , public Gtk::Base
{
public:
    R activate(T* objPtr)
    {
        try
        {
            return (objPtr->*memFunPtr_)();
        }
        catch (...)
        {
            handle_ui_exception();
        }
        return R();
    }

protected:
    typedef R (T::*MemFunPtr)();

    BaseMenuActionImpl(MemFunPtr memFunPtr,
                       const std::string& labelText,
                       const Gtk::BuiltinStockID* stockID)
        : memFunPtr_(memFunPtr)
        , labelText_(labelText)
        , stockID_(stockID)
    { }

    ~BaseMenuActionImpl() throw() { }

    bool add_to(T* obj, Gtk::Menu& menu, MenuClickContext& context)
    {
        if (needLiveThread)
        {
            if (!context.thread() || !context.thread()->is_live())
            {
                return false;
            }
        }
        Gtk::MenuItem* item = manage(Gtk_image_menu_item(stockID_, labelText_));
        Gtk_CONNECT_1(item, activate, this, &BaseMenuActionImpl::activate, obj);
        menu.append(*item);
        item->show_all();

        return true;
    }

private:
    MemFunPtr memFunPtr_;
    std::string labelText_;
    const Gtk::BuiltinStockID* stockID_;
};

#endif // MENU_ACTION_IMPL_H__F6FDEA83_4BBF_4628_B006_3579249427D1

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
