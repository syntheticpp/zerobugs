#ifndef WIDGET_H__2EB89E57_1D25_439D_9729_50FE6D764EE8
#define WIDGET_H__2EB89E57_1D25_439D_9729_50FE6D764EE8
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
////////////////////////////////////////////////////////////////
 #include <gtkmm/widget.h>
 #include <gtkmm/main.h>

namespace Gtk
{
 template<typename W>
 inline void popdown(W* widget)
 {
    Gtk::Main::quit();
 }
}
 #define Gtk_USER_DATA(w) static_cast<void*>((w)->property_user_data())
 #define Gtk_POPDOWN(w) Gtk::Main::quit()

 #define Gtk_WIDTH(w)  (w)->get_width()
 #define Gtk_HEIGHT(w) (w)->get_height()
 #define Gtk_WINDOW(w) (w)->get_window()

 #define show_all_impl show_all_vfunc
 #define delete_event_impl on_delete_event
 #define map_event_impl on_map_event

/*
 struct Overlay : public Gtk::Object {
    template<typename T> Overlay(T arg) : Gtk::Object(arg) { }

    void destroy() { this->destroy_(); Gtk_POPDOWN(this); }
 };

 #define Gtk_DESTROY_SLOT(w) sigc::mem_fun((Overlay*)&(w), &Overlay::destroy)
*/
 #define GTKOBJ(w) (w)->gobj()

#else
////////////////////////////////////////////////////////////////
 #include <gtk--/widget.h>
 #define Gtk_USER_DATA(w) (w)->get_user_data()
 #define Gtk_POPDOWN(w) assert(w); (w)->destroy()

 #define Gtk_WIDTH(w)  (w)->width()
 #define Gtk_HEIGHT(w) (w)->height()

 template<typename W>
 struct WPtr
 {
    W w_;
    W* operator->() { return &w_; }
    const W* operator->() const { return &w_; }

    WPtr(const W& w) : w_(w) { }
 };
 template<typename W>
 inline WPtr<W> wptr(const W& w) { return WPtr<W>(w); }

 #define Gtk_WINDOW(w) wptr((w)->get_window())

 #define GTKOBJ(w) (w)->gtkobj()
#endif

template<typename W>
inline void
Gtk_adjust_coords_for_visibility(const W& w, int& x, int& y)
{
    if (x + Gtk_WIDTH(w) > gdk_screen_width())
    {
        x = gdk_screen_width() - Gtk_WIDTH(w);
    }
    if (y + Gtk_HEIGHT(w) > gdk_screen_height())
    {
        y = gdk_screen_height() - Gtk_HEIGHT(w);
    }
}

#endif // WIDGET_H__2EB89E57_1D25_439D_9729_50FE6D764EE8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
