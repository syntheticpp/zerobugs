#ifndef SLOT_MACROS_H__590E78A1_562E_4E16_A40E_372A63EE4D8D
#define SLOT_MACROS_H__590E78A1_562E_4E16_A40E_372A63EE4D8D
//
// $Id: slot_macros.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

extern "C" void handle_ui_exception() throw();

// Unlike Gtkmm 2 (actually it started with Gtkmm-1.3.4),
// 1.2 does not provide support for global exception handlers.
//
// Use these macros for wrapping slots
//
#if 1
#if GTKMM_2
 #define BEGIN_SLOT_(ret,f,args) ret f args {
 #define CATCH_SLOT_
#else

 #define BEGIN_SLOT_(ret,f,args) ret f args { try
 #define CATCH_SLOT_ catch (...) { handle_ui_exception(); }
#endif

#else
#include "zdk/eh_context_impl.h"
//
//
//
 #define BEGIN_SLOT_(ret,f,args) ret f args { EHContextImpl _ehctxt; try
 #define CATCH_SLOT_ catch (...) { _ehctxt.handle_exception(__PRETTY_FUNCTION__); }
#endif

#define BEGIN_SLOT(f, args) BEGIN_SLOT_(void, f, args)
#define END_SLOT() CATCH_SLOT_; }
#define END_SLOT_(ret) CATCH_SLOT_ return (ret); }

#endif // SLOT_MACROS_H__590E78A1_562E_4E16_A40E_372A63EE4D8D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
