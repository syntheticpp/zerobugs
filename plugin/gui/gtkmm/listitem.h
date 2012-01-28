#ifndef LISTITEM_H__DFB60CF9_933A_49C5_A405_F837E6B91BC7
#define LISTITEM_H__DFB60CF9_933A_49C5_A405_F837E6B91BC7
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

#include "base.h"
#include "signal.h"
#include "zdk/export.h"

#if !defined(GTKMM_2)
////////////////////////////////////////////////////////////////
/* Gtk-- 1.2 */
 #include <gtk--/container.h>
 #include <gtk--/label.h>
 #include <gtk--/listitem.h>

 namespace Gtk
 {
    class List;
 }
 template<typename T>
 void inline ZDK_LOCAL Gtk_list_item_select(Gtk::List&, T iter)
 {
    gtk_list_item_select((*iter)->gtkobj());
 }
 template<typename T>
 void inline ZDK_LOCAL Gtk_list_item_deselect(Gtk::List&, T iter)
 {
    gtk_list_item_deselect((*iter)->gtkobj());
 }
 void inline ZDK_LOCAL Gtk_list_item_select(Gtk::List&, Gtk::ListItem* item)
 {
    gtk_list_item_select(item->gtkobj());
 }
 template<typename T>
 inline std::string ZDK_LOCAL Gtk_list_item_get_text(T item)
 {
    if (const Gtk::Label* label =
            dynamic_cast<const Gtk::Label*>(item->get_child()))
    {
        return label->get_text();
    }
    return std::string();
 }
#else

////////////////////////////////////////////////////////////////
 #include <algorithm> // for std::swap
 #include <gtkmm/widget.h>

 namespace Gtk
 {
    class List;
    class TreeIter;
    class TreeRow;

    class ZDK_LOCAL ListItem : public Widget
    {
        ListItem& operator=(const ListItem& other);
        ListItem(const ListItem& other);

        friend class List;

    public:
        virtual ~ListItem();

        ListItem();

        explicit ListItem(const std::string& value, void* = 0);

        const std::string& value() const { return value_; }

        void set_value(const std::string& value) { value_ = value; }

        void set_user_data(void* data) { userData_ = data; }

        void* user_data() const { return userData_; }

        bool on_button_press_event(GdkEventButton* event);

        ListItem& swap(ListItem& other) throw()
        {
            value_.swap(other.value_);
            std::swap(userData_, other.userData_);
            return *this;
        }

        SigC::Signal1<bool, GdkEventButton*>&
            signal_button_press_event()
        {
            return buttonPress_;
        }

        int index() const { return index_; }

    private:
        std::string value_;
        void* userData_;
        int index_;

        SigC::Signal1<bool, GdkEventButton*> buttonPress_;
    };
 }

 void ZDK_LOCAL Gtk_list_item_select(Gtk::List&, Gtk::ListItem*);

 void ZDK_LOCAL Gtk_list_item_deselect(Gtk::List&, Gtk::ListItem*);

 void ZDK_LOCAL Gtk_list_item_select(Gtk::List&, const Gtk::TreeRow&);

 void ZDK_LOCAL Gtk_list_item_deselect(Gtk::List&, const Gtk::TreeRow&);

 void ZDK_LOCAL Gtk_list_item_select(Gtk::List&, Gtk::TreeIter&);

 void ZDK_LOCAL Gtk_list_item_deselect(Gtk::List&, Gtk::TreeIter&);

 template<typename T>
 inline std::string ZDK_LOCAL Gtk_list_item_get_text(T item)
 {
    return item->value();
 }


#endif

#endif // LISTITEM_H__DFB60CF9_933A_49C5_A405_F837E6B91BC7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
