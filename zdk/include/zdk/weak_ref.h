#ifndef WEAK_REF_H__0F9F3537_46C9_4C7C_89F4_4FAEFB8FEEC5
#define WEAK_REF_H__0F9F3537_46C9_4C7C_89F4_4FAEFB8FEEC5
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

DECLARE_ZDK_INTERFACE_(WeakRef, struct Unknown)
{
    virtual struct Unknown* get_ptr() volatile const = 0;

    virtual long count() const volatile = 0;

    virtual void add_ref() = 0;

    virtual void release() = 0;
};

#endif // WEAK_REF_H__0F9F3537_46C9_4C7C_89F4_4FAEFB8FEEC5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
