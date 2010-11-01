#ifndef ACCELKEY_H__0ED478B2_8E42_40D1_AE3E_7C3DA1E22C81
#define ACCELKEY_H__0ED478B2_8E42_40D1_AE3E_7C3DA1E22C81
//
// $Id: accelkey.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#ifdef GTKMM_2
 #include <gtkmm/accelkey.h>

 inline guint
 Gtk_accel_key(const Gtk::AccelKey& ak)
 {
    return ak.get_key();
 }

 inline Gdk::ModifierType
 Gtk_accel_mod(const Gtk::AccelKey& ak)
 {
    return ak.get_mod();
 }

 inline Glib::ustring
 Gtk_accel_abbrev(const Gtk::AccelKey& ak)
 {
    return ak.get_abbrev();
 }
#else
 #include <string>
 #include <gtk--/menushell.h>

 using namespace Gtk::Menu_Helpers;

 inline guint
 Gtk_accel_key(AccelKey& ak)
 {
    return ak.key();
 }

 inline GdkModifierType
 Gtk_accel_mod(AccelKey& ak)
 {
    return static_cast<GdkModifierType>(ak.mod());
 }

 inline std::string
 Gtk_accel_abbrev(AccelKey& ak)
 {
    return ak.abrev();
 }
#endif

#endif // ACCELKEY_H__0ED478B2_8E42_40D1_AE3E_7C3DA1E22C81
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
