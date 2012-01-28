#ifndef DEBUG_RTTI_H__795A0696_5A05_4D44_9C88_F2074E162E7F
#define DEBUG_RTTI_H__795A0696_5A05_4D44_9C88_F2074E162E7F
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

bool debug_rtti();

#if DEBUG
 #define IF_DEBUG_RTTI(x) if (debug_rtti()) { x; } else { }
#else
 #define IF_DEBUG_RTTI(x)
#endif

#endif // DEBUG_RTTI_H__795A0696_5A05_4D44_9C88_F2074E162E7F
