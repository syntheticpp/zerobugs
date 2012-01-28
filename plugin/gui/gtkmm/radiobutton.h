#ifndef RADIOBUTTON_H__1119D9B3_CA2A_4122_89BE_0F2013E060D8
#define RADIOBUTTON_H__1119D9B3_CA2A_4122_89BE_0F2013E060D8
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

#include "base.h"
#ifdef GTKMM_2
 #include <gtkmm/radiobutton.h>
 #include <gtkmm/radiobuttongroup.h>
#else
 #include <gtk--/radiobutton.h>
 namespace Gtk
 {
    typedef RadioButton_Helpers::Group RadioButtonGroup;
 }
#endif
#endif // RADIOBUTTON_H__1119D9B3_CA2A_4122_89BE_0F2013E060D8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
