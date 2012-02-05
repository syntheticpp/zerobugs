#ifndef LOCK_H__1063647137
#define LOCK_H__1063647137
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

#include "generic/auto.h"
#include "generic/empty.h"
#include <new>  // std::nothrow_t

struct ZDK_LOCAL TryLock { };

/// Generic lock, assumes that the locked object provides
/// the enter() and leave() methods.
template<typename T>
class ZDK_LOCAL Lock
{
    // non-copyable
    Lock(const Lock&);
    // non-assignable
    Lock& operator=(const Lock&);

    T& mx_;
    bool locked_;

public:
    ~Lock() throw()
    {
        if (locked_) mx_.leave();
    }
    explicit Lock(volatile T& mx) : mx_(const_cast<T&>(mx)), locked_(false)
    {
        mx_.enter();
        locked_ = true;
    }
    explicit Lock(volatile T* mx) : mx_(*const_cast<T*>(mx)), locked_(false)
    {
        mx_.enter();
        locked_ = true;
    }
    Lock(volatile T& mx, TryLock) : mx_(const_cast<T&>(mx)), locked_(false)
    {
        locked_ = mx_.trylock();
    }
    template<typename C>
    Lock(T& mx, C& condition, unsigned long timeout = -1)
        : mx_(mx), locked_(false)
    {
        mx.enter();
        locked_ = true;
        mx.wait(condition, timeout);
    }
    operator bool() const { return locked_; }

    bool holds(const volatile T& mx) const { return &mx_ == &mx; }

    template<typename C>
    void wait(C& condition, long timeout = -1)
    { mx_.wait(condition, timeout); }
};


template<> struct ZDK_LOCAL Lock<Empty>
{
    template<typename U>
    explicit Lock(U) { }

    ~Lock() throw() { }
};


template<typename T>
class ZDK_LOCAL Unlock : public Automatic
{
    T& mx_;
    bool unlocked_;

public:
    explicit Unlock(T& mx) : mx_(mx), unlocked_(mx.leave(std::nothrow))
    { }

    explicit Unlock(volatile T& mx) 
        : mx_(mx)
        , unlocked_(mx.leave(std::nothrow))
    { }

    ~Unlock() { if (unlocked_) mx_.enter(); }
};


template<typename M, typename T>
inline T ZDK_LOCAL lock_and_copy(M& mx, const T& value)
{
    Lock<M> lock(mx);
    return value;
}


template<typename M, typename T>
inline void ZDK_LOCAL lock_and_swap(M& mx, T& lhs, T& rhs)
{
    Lock<M> lock(mx);
    lhs.swap(rhs);
}


#endif // LOCK_H__1063647137
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
