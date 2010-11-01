#ifndef WATCHTYPE_H__11AEABB3_0B40_4B82_87BA_1E64DF789212
#define WATCHTYPE_H__11AEABB3_0B40_4B82_87BA_1E64DF789212
//
// $Id: watchtype.h 723 2010-10-28 07:40:45Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

/**
 * Types of memory watchpoints:
 */
enum WatchType
{
    WATCH_VALUE         = 0,    /* triggered when specified value
                                   is stored at memory location */

    WATCH_WRITE         = 1,    /* triggered on memory writes */
    WATCH_READ_WRITE    = 3
};


/* Relationship operators, for WATCH_VALUE */
enum RelType
{
    EQ,
    NEQ,
    LT,
    LTE,
    GT,
    GTE,
};
#endif // WATCHTYPE_H__11AEABB3_0B40_4B82_87BA_1E64DF789212
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
