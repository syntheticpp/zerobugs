#ifndef TEST_COMMON_H__
#define TEST_COMMON_H__
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

#ifdef assert
 #error assert already defined, <assert.h> already included?
#endif

#if defined (NDEBUG)
 #undef NDEBUG
#endif
#include <assert.h>

#endif// TEST_COMMON_H__
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
