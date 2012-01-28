#ifndef DEBUG_H__9C7F37DC_1245_4646_AF69_4AA4FBE14F09
#define DEBUG_H__9C7F37DC_1245_4646_AF69_4AA4FBE14F09
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
#include "dharma/environ.h"
#include "zdk/export.h"

#if DEBUG
 #include <iostream>

 #define IF_DEBUG(x) { x; }

inline ZDK_LOCAL bool debug_maps()
{
    static const bool debug = env::get_bool("ZERO_DEBUG_MAPS");
    return debug;
}
#else
 #define IF_DEBUG(x)
inline ZDK_LOCAL bool debug_maps() { return false; }
#endif
#endif // DEBUG_H__9C7F37DC_1245_4646_AF69_4AA4FBE14F09
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
