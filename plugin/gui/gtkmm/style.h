#ifndef STYLE_H__091C36E1_5A7B_4CD9_AF2B_22062D933D99
#define STYLE_H__091C36E1_5A7B_4CD9_AF2B_22062D933D99
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

#ifdef GTKMM_2

 #include <gtkmm/style.h>

 typedef Glib::RefPtr<Gtk::Style> StylePtr;

 template<typename T>
 inline void Gtk_set_style(const T& w, StylePtr style)
 {
    w->set_style(style);
 }
#else
 #include <gtk--/style.h>

 typedef Gtk::Style* StylePtr;

 template<typename T>
 inline void Gtk_set_style(const T& w, StylePtr style)
 {
    w->set_style(*style);
 }
#endif

#endif // STYLE_H__091C36E1_5A7B_4CD9_AF2B_22062D933D99
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
