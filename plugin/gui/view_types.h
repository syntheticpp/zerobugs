#ifndef VIEW_TYPES_H__88F2BB87_CB29_4F77_85A6_0D6190F5A557
#define VIEW_TYPES_H__88F2BB87_CB29_4F77_85A6_0D6190F5A557
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

enum ViewType
{
    VIEW_DEFAULT = -1,
    VIEW_SOURCE,
    VIEW_DISASSEMBLED,
    VIEW_MIXED,
};


enum BreakPointState
{
    B_NONE,
    B_ENABLED,
    B_DISABLED,
    B_AUTO
};
#endif // VIEW_TYPES_H__88F2BB87_CB29_4F77_85A6_0D6190F5A557
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
