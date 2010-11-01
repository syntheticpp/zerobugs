#ifndef CURSOR_H__4112F032_4CBD_4E97_B201_BFA120F3264B
#define CURSOR_H__4112F032_4CBD_4E97_B201_BFA120F3264B
//
// $Id: cursor.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "set_cursor.h"
#include "zdk/export.h"

#ifdef GTKMM_2
 #include <gdkmm/cursor.h>
#endif

class ZDK_LOCAL SetCursorInScope
{
public:

#ifdef GTKMM_2
    SetCursorInScope(Gtk::Widget& w, Gdk::CursorType csr) : w_(w)
    {
        set_cursor(w_.get_window(), csr);
    }
#else
    SetCursorInScope(Gtk::Widget& w, GdkCursorType csr) : w_(w)
    {
        set_cursor(w_, csr);
    }
#endif
/*
    SetCursorInScope(Gtk::Widget& w, const Gdk_Cursor& csr)
        : w_(w)
    {
        set_cursor(w_.get_window(), csr);
    }
 */
    ~SetCursorInScope() throw()
    {
        try
        {
            set_cursor(w_, Gdk_FLAG(TOP_LEFT_ARROW));
        }
        catch (...)
        {
        }
    }

private:

    SetCursorInScope(SetCursorInScope&);
    SetCursorInScope& operator=(SetCursorInScope&);

    Gtk::Widget& w_;
};
#endif // CURSOR_H__4112F032_4CBD_4E97_B201_BFA120F3264B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
