#ifndef MISC_H__2397EAB0_97FB_4C82_AFC8_E9C88F99B19E
#define MISC_H__2397EAB0_97FB_4C82_AFC8_E9C88F99B19E
//
// $Id: misc.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/watchtype.h"

enum LookupScope
{
    LOOKUP_LOCAL,
    LOOKUP_ALL,
    LOOKUP_MODULE,  // look in current module only
    LOOKUP_PARAM,   // function parameters only
    LOOKUP_UNIT,    // look in current compilation unit
};

#endif // MISC_H__2397EAB0_97FB_4C82_AFC8_E9C88F99B19E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
