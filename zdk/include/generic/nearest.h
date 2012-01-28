#ifndef NEAREST_H__17DE366F_9256_4F50_8B94_AEFD13C9F426
#define NEAREST_H__17DE366F_9256_4F50_8B94_AEFD13C9F426
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

#include "generic/export.h"


/**
 * return true if v1 is closer to target than v2
 */
template<typename T>
inline bool ZDK_LOCAL compare_nearest(T target, T v1, T v2)
{
    T d1 = (target > v1) ? target - v1 : v1 - target;
    T d2 = (target > v2) ? target - v2 : v2 - target;

    return d1 < d2;
}

/**
 * return the value nearest to target
 */
template<typename T>
inline T ZDK_LOCAL nearest_value(T target, T v1, T v2)
{
    return compare_nearest(target, v1, v2) ? v1 : v2;
}
#endif // NEAREST_H__17DE366F_9256_4F50_8B94_AEFD13C9F426
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
