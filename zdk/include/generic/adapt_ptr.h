#ifndef ADAPT_PTR_H__FD143DB4_7C9D_42EF_9326_6F93B52DC372
#define ADAPT_PTR_H__FD143DB4_7C9D_42EF_9326_6F93B52DC372
//
// $Id: adapt_ptr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include "generic/export.h"


template<typename S, typename T>
class ZDK_LOCAL APtr
{
    boost::shared_ptr<S> p_;

public:
    explicit APtr(boost::shared_ptr<S> p) : p_(p) { }

    T* operator->() const
    {
        BOOST_STATIC_ASSERT(sizeof(S) == sizeof(T));
        return static_cast<T*>(p_.get());
    }

    S& operator*() { assert(p_); return *p_; }
    const S& operator*() const { assert(p_); return *p_; }

    operator const void*() const { return p_.get(); }
};

#endif // ADAPT_PTR_H__FD143DB4_7C9D_42EF_9326_6F93B52DC372
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
