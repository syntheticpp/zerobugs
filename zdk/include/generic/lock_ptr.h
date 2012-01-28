#ifndef LOCK_PTR_H__525A4251_345E_403E_89B0_287427FE0F8B
#define LOCK_PTR_H__525A4251_345E_403E_89B0_287427FE0F8B
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

#include "generic/lock.h"


template<typename T, typename M, typename P = T*>
class ZDK_LOCAL LockedPtr : public Lock<M>
{
public:
    typedef T element_type;

    LockedPtr(P ptr, volatile M& mutex)
        : Lock<M>(mutex)
        , ptr_(const_cast<P&>(ptr))
    { }

    LockedPtr(volatile T* ptr, volatile M& mutex)
        : Lock<M>(mutex)
        , ptr_(const_cast<T*>(ptr))
    { }

    ~LockedPtr() throw() { }

    P get() const { return ptr_; }

    P operator->() const { return get(); }

    T& operator *() { assert(get()); return *get(); }
    const T& operator *() const { assert(get()); return *get(); }

    template<typename U> operator void* ()  const
    {
        return static_cast<void*>(ptr_);
    }

private:
    P ptr_;
};

#endif // LOCK_PTR_H__525A4251_345E_403E_89B0_287427FE0F8B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
