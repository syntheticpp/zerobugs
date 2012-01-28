#ifndef FLAGS_H__E20317FB_A8E7_4D63_B44F_FD5171F3FEAE
#define FLAGS_H__E20317FB_A8E7_4D63_B44F_FD5171F3FEAE
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
 #define Gtk_FLAG(f) Gtk::f
 #define Gdk_FLAG(f) Gdk::f

 #include <gdkmm/types.h>
 #include <gtkmm/enums.h>

 namespace Gdk
 {
    const ModifierType MOD_NONE = (ModifierType)0;
 }
 namespace Gtk
 {
    const AttachOptions ATTACH_NONE = (AttachOptions)0;
 }
#else
// Gtk-1.2
 #define Gtk_FLAG(f) GTK_##f
 #define Gdk_FLAG(f) GDK_##f

 static const int GDK_MOD_NONE = 0;
 static const int GTK_ATTACH_NONE = 0;
#endif

#endif // FLAGS_H__E20317FB_A8E7_4D63_B44F_FD5171F3FEAE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
