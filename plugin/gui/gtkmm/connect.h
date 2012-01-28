#ifndef Gtk_CONNECT_H__F6AC4903_531B_4290_8B98_AD0F9FE72C5E
#define Gtk_CONNECT_H__F6AC4903_531B_4290_8B98_AD0F9FE72C5E
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

#include <sigc++/slot.h>
#ifdef GTKMM_2
 #define Gtk_SLOT sigc::mem_fun
 #define Gtk_PTR_FUN sigc::ptr_fun
 #define Gtk_BIND sigc::bind

 #define Gtk_CONNECT_SLOT(w,s,f) (w)->signal_##s().connect(sigc::ptr_fun(f))

// connect to slot
 #define Gtk_CONNECT(w,s,f) (w)->signal_##s().connect(f)

// The sizeof() trick enforce full type declaration, which prevents
// a sigc++ SEGV issue when compiling with optimizations turned on.
// connect to function with no arguments
 #define Gtk_CONNECT_0(w,s,c,m) \
    sizeof(*c) ? (w)->signal_##s().connect(sigc::mem_fun((c),m)) : sigc::connection()

 #define Gtk_CONNECT_AFTER_0(w,s,c,m) \
    sizeof(*c) ? (w)->signal_##s().connect(sigc::mem_fun((c),m)) : sigc::connection()

// connect to function with 1 argument
 #define Gtk_CONNECT_1(w,s,c,m,arg) sizeof(*c) ? \
    (w)->signal_##s().connect(sigc::bind(sigc::mem_fun((c),m),(arg))) : sigc::connection()

// connect to function with 2 arguments
 #define Gtk_CONNECT_2(w,s,c,m,arg0,arg1) sizeof(*c) ? \
    (w)->signal_##s().connect(sigc::bind(sigc::mem_fun((c),m),(arg0), (arg1))) : \
    sigc::connection()

 #define Gtk_CONNECT_X(w,s,c,m,...) sizeof(*c) ? \
    (w)->signal_##s().connect(sigc::bind(sigc::mem_fun((c),m),__VA_ARGS__)) : \
    sigc::connection()

 #define Gtk_SIGNAL_CONNECT g_signal_connect
 #define Gtk_SIGNAL_DISCONNECT g_signal_disconnect

#else
////////////////////////////////////////////////////////////////
// Gtk-1.2
 #define Gtk_SLOT SigC::slot
 #define Gtk_PTR_FUN SigC::slot
 #define Gtk_BIND bind
// #define activate_item activate

 #define Gtk_CONNECT_SLOT(w,s,f) (w)->s.connect((f).slot())

// connect to slot
 #define Gtk_CONNECT(w,s,f) (w)->s.connect(f)

// connect to function with no arguments
 #define Gtk_CONNECT_0(w,s,c,m) (w)->s.connect(slot((c),m))
 #define Gtk_CONNECT_AFTER_0(w,s,c,m) \
    (w)->s.connect_after(slot((c),m))

// connect to function with 1 argument
 #define Gtk_CONNECT_1(w,s,c,m,arg) \
    (w)->s.connect(bind(slot((c),m),(arg)))

// connect to function with 2 arguments
 #define Gtk_CONNECT_2(w,s,c,m,arg0,arg1) \
    (w)->s.connect(bind(slot((c),m),(arg0), (arg1)))
 #define Gtk_SIGNAL_CONNECT gtk_signal_connect
 #define Gtk_SIGNAL_DISCONNECT gtk_signal_disconnect
#endif
#endif // Gtk_CONNECT_H__F6AC4903_531B_4290_8B98_AD0F9FE72C5E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
