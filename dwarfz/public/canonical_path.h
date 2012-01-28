#ifndef CANONICAL_PATH_H__38A9B2BE_CF13_4909_A2B4_42321474FAA3
#define CANONICAL_PATH_H__38A9B2BE_CF13_4909_A2B4_42321474FAA3
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
#ifndef DWARF_NO_CANONICAL_PATH
#include "dharma/canonical_path.h"
 #define CANONICAL_PATH(p) CanonicalPath(p)
#else
 // faster, but less accurate
 #define CANONICAL_PATH(p) (p)
#endif
#endif // CANONICAL_PATH_H__38A9B2BE_CF13_4909_A2B4_42321474FAA3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
