#ifndef REF_COUNTED_H__1060467356
#define REF_COUNTED_H__1060467356
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

#include "zdk/atomic.h"
#include "zdk/export.h"
#include "zdk/unknown2.h"
#include "zdk/weak_ref.h"



/**
 * Base class for intrusively reference-counted objects.
 * @note the methods that modify the reference count are
 * intentionally protected, so that they are not called
 * by client code directly, but by the RefPtr smart pointer.
 */
DECLARE_ZDK_INTERFACE_(RefCounted, struct Unknown)
{
    template<typename T> friend class RefPtr;
    template<typename T> friend class WeakPtr;

public:
    enum { supports_weak_ref = true };
    virtual long ref_count() const volatile = 0;

protected:
    virtual void inc_ref() = 0;
    virtual bool dec_ref_and_test() = 0;
    virtual void release() = 0;

    virtual WeakRef* weak_ref() const volatile = 0;
};



DECLARE_ZDK_INTERFACE(RefTracker)
{
    virtual void register_object(RefCounted*) = 0;
};

#endif // REF_COUNTED_H__1060467356
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
