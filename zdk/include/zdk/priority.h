#ifndef PRIORITY_H__93C954DC_E728_4DDA_800D_0F1F62E6A15A
#define PRIORITY_H__93C954DC_E728_4DDA_800D_0F1F62E6A15A
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

#include "zdk/unknown2.h"


DECLARE_ZDK_INTERFACE_(Priority, struct Unknown)
{
    DECLARE_UUID("01ac048e-68b8-4b6f-9ba8-4fb7849bfbb1")

    enum Class
    {
        EXPERIMENTAL = -1,
        LOW,
        NORMAL,
        HIGH,
    };

    virtual Class priority_class() const = 0;
};


#endif // PRIORITY_H__93C954DC_E728_4DDA_800D_0F1F62E6A15A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
