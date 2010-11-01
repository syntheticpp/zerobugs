
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#ifndef GTKMM_2
 #include <gtk--/menubar.h>
  typedef Gtk::MenuBar ZMenuBar;

#else
 #include <gtkmm/menubar.h>

 struct ZMenuBar : public Gtk::MenuBar
 {
 };
#endif

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
