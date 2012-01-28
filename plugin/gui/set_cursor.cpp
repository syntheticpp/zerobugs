// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "gtkmm/window.h"
#include "set_cursor.h"
#include "dharma/environ.h"

#if GTKMM_2
 static bool changeCsr = !env::get_bool("ZERO_NO_BUSY_CURSOR", false);

 void set_cursor(Glib::RefPtr<Gdk::Window> w, Gdk::CursorType csr)
 {
    if (changeCsr && w)
    {
        w->set_cursor(Gdk::Cursor(csr));
    }
 }
#else
// gtk-- 1.2
 void set_cursor(Gtk::Widget& w, GdkCursorType csr)
 {
    static bool changeCsr = !env::get_bool("ZERO_NO_BUSY_CURSOR", false);
    if (changeCsr && w.is_realized())
    {
        w.get_window().set_cursor(csr);
    }
 }
#endif
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
