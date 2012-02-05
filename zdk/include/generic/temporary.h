#ifndef TEMPORARY_H__1059368586
#define TEMPORARY_H__1059368586
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
#include <cstdlib>
#include "empty.h"
#include "lock.h"

/**
 *  Helper class for modifying a value temporarily, and have it
 *  automatically restore when the scope is exited
 *  Example:
 *     int some_value = 42;
 *     ...
 *     {
 *          Temporary<int> set_value(some_value, 13);
 *          assert(some_value == 13);
 *          ...
 *          ...
 *     }
 *     assert(some_value == 42);
 */
template<typename T, typename M = Empty>
class ZDK_LOCAL Temporary
{
    // non-copyable, non-assignable
    Temporary(const Temporary&);
    Temporary& operator=(const Temporary&);

    volatile T& ref_;
    T value_;
    volatile M* mutex_;

public:
    typedef T value_type;
    typedef T& reference;

    explicit Temporary(volatile T& ref)
        : ref_(ref), value_(ref), mutex_(NULL)
    { }

    template<typename U>
    Temporary(volatile T& ref, U tempValue)
        : ref_(ref), value_(const_cast<T&>(ref)), mutex_(NULL)
    {
        const_cast<T&>(ref_) = tempValue;
    }

    template<typename U>
    Temporary(volatile T& ref, U tempValue, volatile M* mutex)
        : ref_(ref), value_(const_cast<T&>(ref)), mutex_(mutex)
    {
        Lock<M> lock(mutex_);
        const_cast<T&>(ref_) = tempValue;
    }

    ~Temporary() throw()
    {
        Lock<M> lock(mutex_);
        const_cast<T&>(ref_) = value_;
    }

    T operator()() { return value_; }

    /**
     * manually set the value that will be assigned to ref
     */
    void set_value(T value) { value_ = value; }
};

#endif // TEMPORARY_H__1059368586
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
