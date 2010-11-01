// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: file_selection.cpp 714 2010-10-17 10:03:52Z root $
//
#include "file_selection.h"
#include <iostream>

#ifdef GTKMM_2
FileSel::FileSel(const char* path, Glib::RefPtr<Gdk::Pixmap> icon)
    : SuperClass(path)
{
}


bool FileSel::on_delete_event(GdkEventAny* event)
{
    signal_destroy_();
    bool result = SuperClass::on_delete_event(event);
    return result;
}

#else // Gtk-- 1.2

FileSel::FileSel(const char* path, Gdk_Pixmap& icon)
    : OnMapEventImpl<Gtk::FileSelection>(path)
{
}


#endif // gtk-- 1.2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
