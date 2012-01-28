#ifndef ACCESS_H__B5EAAF64_A60B_4DEC_9C2B_30D512E97939
#define ACCESS_H__B5EAAF64_A60B_4DEC_9C2B_30D512E97939
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

#include <dwarf.h>

namespace Dwarf
{
    enum Access
    {
        a_public = DW_ACCESS_public,
        a_protected = DW_ACCESS_protected,
        a_private = DW_ACCESS_private,
    };
}
#endif // ACCESS_H__B5EAAF64_A60B_4DEC_9C2B_30D512E97939
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
