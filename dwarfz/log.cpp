//
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

#include <cstdlib>
#include "private/log.h"


int Dwarf::debug_verbosity()
{
    static int verbosity = 0;
    if (verbosity == 0)
    {
        if (const char* tmp = getenv("ZERO_DEBUG_DWARF"))
        {
            verbosity = strtol(tmp, 0, 0);
        }
    }
    return verbosity;
}



// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
