#ifndef BUFFER_H__59054C96_E219_46F6_A7A5_5AB5078E9948
#define BUFFER_H__59054C96_E219_46F6_A7A5_5AB5078E9948
//
// $Id: buffer.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zobject.h"


DECLARE_ZDK_INTERFACE_(Buffer, ZObject)
{
    DECLARE_UUID("f7e7aca7-a182-4747-9b3e-6a69e7a6a095")

    virtual void resize(size_t) = 0;

    virtual size_t size() const = 0;

    virtual const char* data() const = 0;

    virtual void put(const void*, size_t, size_t = 0) = 0;
};
#endif // BUFFER_H__59054C96_E219_46F6_A7A5_5AB5078E9948
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
