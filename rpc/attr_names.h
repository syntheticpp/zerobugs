#ifndef ATTR_NAMES_H__1D2700DD_9795_466C_A01B_9B288B153213
#define ATTR_NAMES_H__1D2700DD_9795_466C_A01B_9B288B153213
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
#define DECL_RPC_ATTR(a)    #a ,

namespace RPC
{
    const char* const attr_name[] =
    {
        #include "rpc/attr_def.h"
    };
}
#undef DECL_RPC_ATTR
#endif // ATTR_NAMES_H__1D2700DD_9795_466C_A01B_9B288B153213
