#ifndef MAIN_H__352B08DD_884B_4644_AE48_43496678A9B2
#define MAIN_H__352B08DD_884B_4644_AE48_43496678A9B2
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

#if !defined( GTKMM_2 )
 #include <gtk--/main.h>

 #define GLIB_SIGNAL_IO Gtk::Main::input
 #define GLIB_SIGNAL_IDLE Gtk::Main::idle

#else
 #include <glibmm/main.h>
 #include <gtkmm/main.h>

 #define GDK_INPUT_READ Glib::IO_IN

 #define GLIB_SIGNAL_IO Glib::signal_io()
 #define GLIB_SIGNAL_IDLE Glib::signal_idle()

#endif
#endif // MAIN_H__352B08DD_884B_4644_AE48_43496678A9B2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
