#ifndef SET_CURSOR_H__CBD91E0C_4EBF_4300_97DD_7BC1AFC6F5A5
#define SET_CURSOR_H__CBD91E0C_4EBF_4300_97DD_7BC1AFC6F5A5
//
// $Id: set_cursor.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if GTKMM_2
 #include "gtkmm/flags.h"
 #include "gtkmm/widget.h"
 #include <gdkmm/cursor.h>

 namespace Gdk
 {
    class Window;
 }
 typedef Gdk::Cursor Gdk_Cursor;

 void set_cursor(Glib::RefPtr<Gdk::Window>, Gdk::CursorType);

 inline void set_cursor(Gtk::Widget& w, Gdk::CursorType csr)
 {
    set_cursor(w.get_window(), csr);
 }

#else
 #include <gdk--/cursor.h>

 class Gdk_Cursor;
 class Gdk_Window;

 void set_cursor(Gtk::Widget&, GdkCursorType);

#endif
#endif // SET_CURSOR_H__CBD91E0C_4EBF_4300_97DD_7BC1AFC6F5A5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
