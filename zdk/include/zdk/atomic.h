#ifndef ATOMIC_H__043C4CAF_6083_4150_9F1A_7038D205A2CA
#define ATOMIC_H__043C4CAF_6083_4150_9F1A_7038D205A2CA
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

#include "zdk/config.h"
#include "zdk/export.h"
#include <assert.h>

#if defined(__APPLE__)
 #include "zdk/atomic-apple.h"
#else
 #include <boost/static_assert.hpp>
 #include <atomic_ops.h>

#define ZDK_BARRIER AO_nop_full

struct atomic_t
{
    volatile AO_t counter;
    explicit atomic_t(AO_t i = 0) : counter(i) { }
};

/*
void inline ZDK_LOCAL atomic_set(volatile atomic_t& v, AO_t i)
{
    v.counter = i;
}
*/


inline AO_t ZDK_LOCAL atomic_read(const volatile atomic_t& v)
{
    //AO_nop_full();
    return v.counter;
}


/**
 * Atomically increment v
 */
inline void ZDK_LOCAL atomic_inc(volatile atomic_t& v)
{
    AO_fetch_and_add1(&v.counter);
}


/**
 * Atomically decrement v
 */
inline void ZDK_LOCAL atomic_dec(volatile atomic_t& v)
{
    AO_fetch_and_sub1(&v.counter);
}

/**
 * Decrement and return true if reached zero.
 */
inline bool ZDK_LOCAL atomic_dec_and_test(volatile atomic_t& v)
{
    AO_t old = AO_fetch_and_sub1_full(&v.counter);

    assert(old > 0);
    return old == 1;
}

template<typename T>
struct ZDK_LOCAL CompareAndSwap
{
    // implicit interface
    bool operator()(volatile T* val, T oldVal, T newVal);
};


template<>
struct ZDK_LOCAL CompareAndSwap<AO_t>
{
    inline bool
    operator()(volatile AO_t* val, AO_t oldVal, AO_t newVal) const
    {
        return AO_compare_and_swap_full(val, oldVal, newVal) != 0;
    }
};

template<typename T>
struct ZDK_LOCAL CompareAndSwap<T*>
{
    inline bool operator()(T* volatile* val, T* oldVal, T* newVal) const
    {
        BOOST_STATIC_ASSERT(sizeof (T*) == sizeof (AO_t));

        return AO_compare_and_swap_full((volatile AO_t*)val,
                                        (AO_t)(oldVal),
                                        (AO_t)newVal) != 0;
    }
};

/**
 * @return true if val was equal to oldval, and has been set to newval,
 * or false if val was NOT equal to the oldval
 */
template<typename T>
inline bool ZDK_LOCAL compare_and_swap(volatile T* val, T oldVal, T newVal)
{
    return CompareAndSwap<T>()(val, oldVal, newVal);
}

inline bool ZDK_LOCAL atomic_test(volatile atomic_t& v)
{
    return !compare_and_swap(&v.counter, (AO_t)0, (AO_t)0);
}

inline bool ZDK_LOCAL atomic_test(const volatile atomic_t& v)
{
    return !compare_and_swap(&const_cast<volatile atomic_t&>(v).counter, (AO_t)0, (AO_t)0);
}


#endif

#endif // ATOMIC_H__043C4CAF_6083_4150_9F1A_7038D205A2CA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
