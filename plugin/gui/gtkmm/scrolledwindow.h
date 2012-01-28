#ifndef SCROLLEDWINDOW_H__3A0E50C9_61B9_4A7A_86E9_6A8A21DADC1F
#define SCROLLEDWINDOW_H__3A0E50C9_61B9_4A7A_86E9_6A8A21DADC1F
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
 #include <gtkmm/scrolledwindow.h>
 //#define add_with_viewport(sw,w) (sw)->add(w)

inline void Gtk_add_with_viewport(Gtk::ScrolledWindow* sw, Gtk::Widget& w)
{
    sw->add(w);
}
#else
 #include <gtk--/scrolledwindow.h>
 //#define add_with_viewport(sw,w) (sw)->add_with_viewport(w)
inline void Gtk_add_with_viewport(Gtk::ScrolledWindow* sw, Gtk::Widget& w)
{
    sw->add_with_viewport(w);
}
#endif
#endif // SCROLLEDWINDOW_H__3A0E50C9_61B9_4A7A_86E9_6A8A21DADC1F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
