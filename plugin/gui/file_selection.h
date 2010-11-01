#ifndef FILESEL_H__DF40D424_8739_4A53_9F28_1EAC339E1FB6
#define FILESEL_H__DF40D424_8739_4A53_9F28_1EAC339E1FB6
//
// $Id: file_selection.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "gtkmm/connect.h"
#include "gtkmm/fileselection.h"
#include "gtkmm/icon_mapper.h"
#include "gtkmm/main.h"
#include "zdk/export.h"

/**
 * Customized File Selection dialog -- currently just a
 * workaround for the icon not being inherited (at least in Gtk-1.2)
 */
#ifdef GTKMM_2
#include <gtkmm/filechooserdialog.h>

 class ZDK_LOCAL FileSel : public OnMapEventImpl<Gtk::FileSelection>
 {
     typedef OnMapEventImpl<Gtk::FileSelection> SuperClass;

public:
    FileSel(const char*, Glib::RefPtr<Gdk::Pixmap> icon);

    sigc::signal<void> signal_destroy() { return signal_destroy_; }

private:
    bool on_delete_event(GdkEventAny*);

    sigc::signal<void> signal_destroy_;
 };

#else

 class FileSel : public OnMapEventImpl<Gtk::FileSelection>
 {
public:
    FileSel(const char*, Gdk_Pixmap& icon);
 };
#endif // GTKMM_2



void inline ZDK_LOCAL close_on_cancel(FileSel& fsel)
{
#ifdef GTKMM_2
    Gtk_CONNECT_SLOT(fsel.get_cancel_button(), clicked, Gtk::Main::quit);
#else
    Gtk_CONNECT_SLOT(fsel.get_cancel_button(), clicked, fsel.destroy);
#endif
}


#endif // FILESEL_H__DF40D424_8739_4A53_9F28_1EAC339E1FB6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
