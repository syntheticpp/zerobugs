#ifndef ZOBJECT_H__CBCA4021_2AED_4036_9026_114E590D4166
#define ZOBJECT_H__CBCA4021_2AED_4036_9026_114E590D4166
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

#include "zdk/ref_counted.h"
#include "zdk/unknown2.h"

/**
 * Combine reference-counting with interface querying
 */
struct ZDK_LOCAL ZObject : public RefCounted, public Unknown2
{
    DECLARE_UUID("5ae08189-3942-408d-bd9f-b9bfc2cc6be9")
};


DECLARE_ZDK_INTERFACE_(ZObjectManager, Unknown2)
{
    DECLARE_UUID("5620e8a2-5b16-4a8e-99b1-623bec0c0637")

    virtual void manage(ZObject*) = 0;
};

#endif // ZOBJECT_H__CBCA4021_2AED_4036_9026_114E590D4166
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
