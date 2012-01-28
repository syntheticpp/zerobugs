#ifndef DIALOG_H__237DBF08_7D47_46EC_B26D_16546A7D8449
#define DIALOG_H__237DBF08_7D47_46EC_B26D_16546A7D8449
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
 #include <gtkmm/dialog.h>
 #include <gtkmm/stock.h>
 #include <gtk/gtkstock.h>

 #define Gtk_STOCK_ID(n) Gtk::StockID(n)
#else
 #include <gtk--/dialog.h>
 #define Gtk_STOCK_ID(n) (n)
#endif
#endif // DIALOG_H__237DBF08_7D47_46EC_B26D_16546A7D8449
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
