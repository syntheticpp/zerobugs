#ifndef ACCELMAP_H__1A6B2C95_C6E3_45A3_A3ED_0382319BBA7E
#define ACCELMAP_H__1A6B2C95_C6E3_45A3_A3ED_0382319BBA7E
//
// $Id: accelmap.h 716 2010-10-17 22:16:32Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <map>
#include <string>
#include "generic/singleton.h"
#include "gtkmm/menuitem.h"
#include "gtkmm/menushell.h"

#ifdef GTKMM_2
#include <gtk/gtkaccelmap.h>
#include <gtkmm/accelmap.h>
#include <gtkmm/menu.h>
#include "zdk/export.h"

 typedef Glib::RefPtr<Gtk::MenuItem> MenuItemPtr;
#else

 #include "accelkey.h"

 typedef Gtk::MenuItem* MenuItemPtr;
#endif


class MenuAccelMap
{
    typedef std::map<std::string, MenuItemPtr> map_type;

    map_type map_;

public:
    typedef map_type::const_iterator const_iterator;

    void put(const std::string& path, MenuItemPtr item)
    {
        map_[path] = item;
    }
    MenuItemPtr get(const std::string& path) const
    {
        map_type::const_iterator i = map_.find(path);
        return i == map_.end() ? MenuItemPtr(0) : i->second;
    }

    const_iterator begin() const { return map_.begin(); }
    const_iterator end() const { return map_.end(); }
};

typedef Singleton<MenuAccelMap> TheAccelMap;


#ifdef GTKMM_2

 namespace Gtk
 {
    namespace AccelMap
    {
        inline bool ZDK_LOCAL
        change_entry(const std::string& path, const AccelKey& ak)
        {
         /*
            if (MenuItemPtr item = TheAccelMap::instance().get(path))
            {
                Gtk::Menu& menu = dynamic_cast<Gtk::Menu&>(*item->get_parent());

                if (Glib::RefPtr<AccelGroup> grp = menu.get_accel_group())
                {
                    GtkAccelKey oldKey;
                    if (gtk_accel_map_lookup_entry(path.c_str(), &oldKey))
                    {
                        // grp->disconnect_key(oldKey.accel_key,
                         //                   Gdk::ModifierType(oldKey.accel_mods));
                        item->remove_accelerator(grp, oldKey.accel_key,
                                            Gdk::ModifierType(oldKey.accel_mods));
                    }
                }
            }
          */
            return change_entry(path, ak.get_key(), ak.get_mod(), true);
        }
    }
 }

 inline void ZDK_LOCAL
 Gtk_set_accel_path(Gtk::Menu_Helpers::Element& elem,
                    const std::string& path)
 {
    TheAccelMap::instance().put(path, elem.get_child());
    elem.get_child()->set_accel_path(path);
 }

#else
 #include <gtk--/accelgroup.h>
 #include <gtk--/button.h>
 #include <gtk--/label.h>
 #include <gtk--/window.h>


namespace Gtk
{
    namespace AccelMap
    {
        typedef Menu_Helpers::MenuElem BaseMenuElem;
        using namespace Menu_Helpers;

        struct MenuElement : public BaseMenuElem
        {
            explicit MenuElement(MenuItem& item)
                : BaseMenuElem(item)
            { }
            explicit MenuElement(BaseMenuElem& elem)
                : BaseMenuElem(elem)
            { }

            void set_path(const std::string& path)
            {
                TheAccelMap::instance().put(path, get_child());
            }

            void set_accel(AccelKey& ak)
            {
                if (MenuItem* item = get_child())
                {
                    assert(item->get_toplevel()->get_accel_group());

                    item->remove_accelerators("activate", true);

                    item->add_accelerator("activate",
                        *item->get_toplevel()->get_accel_group(),
                         ak.key(), ak.mod(),
                         GtkAccelFlags(1));

                    item->accel_key = ak.key_;

                    item->accelerate();
                    item->show_accel_label();

                    if (item->accel_label_)
                    {
                        string accel = ak.abrev();
                        item->accel_label_->set_text(accel);
                    }
                }
            }
        };

        inline bool
        change_entry(const std::string& path, AccelKey& ak)
        {
            if (MenuItem* item = TheAccelMap::instance().get(path))
            {
                MenuElement(*item).set_accel(ak);
                return true;
            }
            return false;
        }
    };
}

inline void ZDK_LOCAL
Gtk_set_accel_path(Gtk::Menu_Helpers::MenuElem& elem,
                    const std::string& path)
{
    Gtk::AccelMap::MenuElement(elem).set_path(path);
}
#endif
#endif // ACCELMAP_H__1A6B2C95_C6E3_45A3_A3ED_0382319BBA7E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
