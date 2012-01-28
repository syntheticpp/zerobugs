#ifndef WATCH_H__09A4AAA3_CC37_4C51_BD81_AAAD8CE523AA
#define WATCH_H__09A4AAA3_CC37_4C51_BD81_AAAD8CE523AA
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
#include "zdk/enum.h"
#include "zdk/zobject.h"

DECLARE_ZDK_INTERFACE_(WatchList, ZObject)
{
    DECLARE_UUID("83d9286f-62b9-4ca2-8624-59f0f0361431")

    virtual bool add(const char*) = 0;

    virtual void clear() = 0;

    virtual size_t foreach(EnumCallback<const char*>*) const = 0;
};

// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
#endif // WATCH_H__09A4AAA3_CC37_4C51_BD81_AAAD8CE523AA
