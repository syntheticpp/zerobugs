#ifndef LABEL_H__85EC81A4_DEF0_43C7_B2D6_2BF18DB95B3C
#define LABEL_H__85EC81A4_DEF0_43C7_B2D6_2BF18DB95B3C
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

#include "zdk/check_ptr.h"

#ifdef GTKMM_2
 #include "gtkmm/label.h"

 inline guint Gtk_get_mnemonic_keyval(Gtk::Label* lbl)
 {
    CHKPTR(lbl)->set_use_underline(true);
    int accelKey = CHKPTR(lbl)->get_mnemonic_keyval();
    if (accelKey < 0)
    {
        return 0;
    }
    return accelKey;
 }
#else
 #include <gtk--/label.h>

 inline guint Gtk_get_mnemonic_keyval(Gtk::Label* lbl)
 {
    return CHKPTR(lbl)->parse_uline(lbl->get_text());
 }
#endif

#endif // LABEL_H__85EC81A4_DEF0_43C7_B2D6_2BF18DB95B3C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
