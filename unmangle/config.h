#ifndef CONFIG_H__85602C72_C754_40BC_AFA1_348F0D8474A1
#define CONFIG_H__85602C72_C754_40BC_AFA1_348F0D8474A1
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
//
// comment this out if you prefer std::basic_string<char, ... etc.
// to std::string
#define USE_SHORT_STD_SUBST

//#define USE_STD_CONTAINERS

// attempt to workaround ABI bug in g++ 3.2.x --
// warning: it may do more harm than good in some cases
//#define WORKAROUND_GNU_ABI_BUG

#ifndef _ISOC99_SOURCE
 #define _ISOC99_SOURCE 1 // turn on strtold
#endif

#include <stdlib.h>

#endif // CONFIG_H__85602C72_C754_40BC_AFA1_348F0D8474A1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
