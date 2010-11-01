#ifndef MENU_H__6130136C_31B3_40A4_9452_EB7BF696B2A7
#define MENU_H__6130136C_31B3_40A4_9452_EB7BF696B2A7
//
// $Id: menu.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if GTKMM_2
 #include <gtkmm/menu.h>

 #define Gtk_MENU_ELEM(t,k,s) MenuElem(Glib::ustring(t),Gtk::AccelKey(k),(s))
 #ifndef Gtk_MENU_ITEM
  #define Gtk_MENU_ITEM(mi) (mi)
 #endif

#else

 #include <gtk--/menu.h>
 #define Gtk_MENU_ELEM(t,k,s) MenuElem((t),(k),(s))

 #ifndef Gtk_MENU_ITEM
  #define Gtk_MENU_ITEM(mi) (*(mi))
 #endif
#endif

#endif // MENU_H__6130136C_31B3_40A4_9452_EB7BF696B2A7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
