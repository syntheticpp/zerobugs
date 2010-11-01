#ifndef MENUITEM_H__B430328A_4CE2_4A4B_A901_C27567F5C225
#define MENUITEM_H__B430328A_4CE2_4A4B_A901_C27567F5C225
//
// $Id: menuitem.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "box.h"
#include "label.h"
#include "stock.h"

#ifdef GTKMM_2
 #include <gtkmm/image.h>
 #include <gtkmm/menuitem.h>
 #include <gtkmm/menushell.h>
 #include <gtkmm/imagemenuitem.h>

 #ifndef Gtk_MENU_ITEM
  #define Gtk_MENU_ITEM(mi) (mi)
 #endif

 inline Gtk::MenuItem*
 Gtk_image_menu_item(const Gtk::BuiltinStockID* stockID, const std::string& label, Gtk::AccelKey k = Gtk::AccelKey())
 {
    // hack, to access set_accel_key
    struct Overlay : public Gtk::MenuItem
    {
        void set_accel_key(Gtk::AccelKey k)
        {
            Gtk::MenuItem::set_accel_key(k);
        }
    };

    Gtk::MenuItem* item = NULL;
    if (stockID)
    {
        Gtk::Image* img = new Gtk::Image(*stockID, Gtk::ICON_SIZE_MENU);
        item = new Gtk::ImageMenuItem(*img, label, true);
    }
    else
    {
        item = new Gtk::MenuItem(label, true);
    }
    if (!k.is_null())
    {
        static_cast<Overlay*>(item)->set_accel_key(k);
    }
    return item;
 }

/*
 inline void
 Gtk_set_accel_path(Gtk::Menu_Helpers::Element& elem,
                    const std::string& path)
 {
    TheAccelMap::instance().put(path, elem.get_child());
    elem.get_child()->set_accel_path(path);
 }
 */

 template<typename M>
 inline Gtk::Label* Gtk_get_label(M item)
 {
    if (!item)
        return NULL;
    //std::clog << "*** " << typeid(*item->get_child()).name() << " ***\n";
    return dynamic_cast<Gtk::AccelLabel*>(item->get_child());
}

#else
 #include <gtk--/menuitem.h>

 #ifndef Gtk_MENU_ITEM
  #define Gtk_MENU_ITEM(mi) (*(mi))
 #endif

 inline Gtk::MenuItem*
 Gtk_image_menu_item(const Gtk::BuiltinStockID*, const std::string& label, Gtk::AccelKey = "")
 {
    return new Gtk::MenuItem(label);
 }

/*
 inline void
 Gtk_set_accel_path(Gtk::Menu_Helpers::MenuElem& elem,
                    const std::string& path)
 {
    Gtk::AccelMap::MenuElement(elem).set_path(path);
 }
*/

 inline Gtk::Label* Gtk_get_label(Gtk::MenuItem* item)
 {
    if (!item)
        return NULL;
    // std::clog << "*** " << typeid(*item->get_child()).name() << " ***\n";
    if (Gtk::Box* box = dynamic_cast<Gtk::Box*>(item->get_child()))
    {
        Gtk::Box_Helpers::BoxList& boxList = box->children();
        return boxList.empty()
            ? 0
            : dynamic_cast<Gtk::Label*>((*boxList.begin())->get_widget());
    }
    return 0;
 }
#endif

#endif // MENUITEM_H__B430328A_4CE2_4A4B_A901_C27567F5C225
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
