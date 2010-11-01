#ifndef RESIZE_H__73F9B6FC_5663_4CC6_93B7_FC78D6780B02
#define RESIZE_H__73F9B6FC_5663_4CC6_93B7_FC78D6780B02
//
// $Id: resize.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/check_ptr.h"

#ifdef GTKMM_2
/*** Gtkmm 2.x ***/
 #include <gdkmm/window.h>

 template<typename T>
 void inline Gtk_set_size(T w, int width, int height)
 {
 /*
    Glib::RefPtr<Gdk::Window> wnd = CHKPTR(w)->get_window();
    if (wnd)
    {
        wnd->resize(width, height);
    }
    else
 */
    {
        w->set_size_request(width, height);
    }
 }

 template<typename T>
 void inline Gtk_move(T w, int x, int y)
 {
    w->move(x, y);
 }

 template<typename T>
 void inline Gtk_set_resizable(T w, bool resizable)
 {
    w->set_resizable(resizable);
 }


 template<typename T>
 void inline Gtk_set_resizable(const std::auto_ptr<T>& w, bool resizable)
 {
    w->set_resizable(true);
 }

#else
 #include <memory>

/*** Gtkmm 1.2 ***/
 template<typename T>
 void inline Gtk_set_size(T w, int width, int height)
 {
    CHKPTR(w)->set_usize(width, height);
 }

 template<typename T>
 void inline Gtk_move(T w, int x, int y)
 {
    w->set_uposition(x, y);
 }

 template<typename T>
 void inline Gtk_set_resizable(T w, bool can_grow)
 {
    w->set_policy(false, can_grow, false);
 }

 template<typename T>
 void inline Gtk_set_resizable(const std::auto_ptr<T>& w, bool can_grow)
 {
    w->set_policy(false, can_grow, false);
 }
#endif

// Shoud not link -- it is an error to sink the auto_ptr
 template<typename T>
 void Gtk_set_size(std::auto_ptr<T>, int width, int height);

 template<typename T>
 void inline Gtk_move(std::auto_ptr<T>, int x, int y);


#endif // RESIZE_H__73F9B6FC_5663_4CC6_93B7_FC78D6780B02
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
