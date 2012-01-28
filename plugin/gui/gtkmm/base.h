#ifndef BASE_H__F57F9207_F726_4348_ACBC_2C7A2AF84A12
#define BASE_H__F57F9207_F726_4348_ACBC_2C7A2AF84A12
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

#include <boost/static_assert.hpp>

/* Ideally we should use GTK_MAJOR_VERSION, but there's a
   chicken and egg problem here: we don't know what header
   files to include unless we know if dealing with 1.2, or 2.x Gtk */

#if !defined (GTKMM_2) && ((GTKMM == 2) || (GTK_MAJOR_VERSION == 2))
 #define GTKMM_2 1
#endif

#if !GTKMM_2
 #include <gtk/gtkfeatures.h>
 #include <gtk--/base.h>
 BOOST_STATIC_ASSERT(GTK_MAJOR_VERSION == 1);
#else
 #include <gtk/gtkversion.h>
 #include <gtkmm/base.h>
 #include <sigc++/sigc++.h>

 BOOST_STATIC_ASSERT(GTK_MAJOR_VERSION == 2);

 namespace Gtk
 {
    class Base { };
 }
#endif
#endif // BASE_H__F57F9207_F726_4348_ACBC_2C7A2AF84A12
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
