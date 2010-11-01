// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: string.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/string.h"
#define __USE_GNU
#include <ctype.h>


int strcmp_ignore_space(
    const char* __restrict p,
    const char* __restrict r
    )
{
    for (; *p; ++p)
    {
        if (isspace(*p))
        {
            continue;
        }
        else
        {
            if (*r == 0)
            {
                return 1; // p > r
            }
            while (isspace(*r))
            {
                ++r;
            }
        }
        if (*p < *r)
        {
            return -1;
        }
        if (*p > *r)
        {
            return 1;
        }
        if (*r)
        {
            ++r;
        }
    }
    while (isspace(*r))
    {
        ++r;
    }
    return *r == 0 ? 0 : -1;
}

