#ifndef ACCELGROUP_H__0C5FA203_2FAE_4168_8DBB_8765E55A583E
#define ACCELGROUP_H__0C5FA203_2FAE_4168_8DBB_8765E55A583E
//
// $Id: accelgroup.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#ifdef GTKMM_2
 #include <gtkmm/accelgroup.h>
 typedef Glib::RefPtr<Gtk::AccelGroup> Gtk_ACCEL_GROUP_PTR;

#else
 #include <gtk--/accelgroup.h>
 typedef Gtk::AccelGroup* Gtk_ACCEL_GROUP_PTR;

#endif
#include "add_accel.h"

#endif // ACCELGROUP_H__0C5FA203_2FAE_4168_8DBB_8765E55A583E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
