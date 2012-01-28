#ifndef SIGNAL_H__FA8B0DA0_D2E3_4673_9062_9748A3EF35D0
#define SIGNAL_H__FA8B0DA0_D2E3_4673_9062_9748A3EF35D0
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
 #define LIBSIGC_DISABLE_DEPRECATED 1
 #include <sigc++/signal.h>
 #define Signal0 signal0
 #define Signal1 signal1
 #define Signal2 signal2
 #define Signal3 signal3
 #define Signal4 signal4
 #define Connection connection
 #define slot() make_slot()
 #define SigC sigc
 //namespace SigC = sigc;
#else
 #include <sigc++/basic_signal.h>
#endif

#endif // SIGNAL_H__FA8B0DA0_D2E3_4673_9062_9748A3EF35D0
