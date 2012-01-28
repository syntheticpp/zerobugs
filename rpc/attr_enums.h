#ifndef ATTR_ENUMS_H__2E0578C6_C42C_4A37_966C_8914890C5463
#define ATTR_ENUMS_H__2E0578C6_C42C_4A37_966C_8914890C5463
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
//
#define DECL_RPC_ATTR(a)    a,

namespace RPC
{
    enum AttrEnum
    {
        #include "rpc/attr_def.h"
    };
}
#undef DECL_RPC_ATTR

#endif // ATTR_ENUMS_H__2E0578C6_C42C_4A37_966C_8914890C5463
