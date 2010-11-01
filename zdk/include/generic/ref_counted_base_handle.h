#ifndef REF_COUNTED_BASE_HANDLE_H__1060467395
#define REF_COUNTED_BASE_HANDLE_H__1060467395
//
// $Id: ref_counted_base_handle.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// From my C/C++ Users Journal (Aug 2001 issue) article
//  "Generalizing The Concept of auto_ptr"
//
#include <cassert>
#include <algorithm>                // for std::swap
#include "auto_handle.h"

/**
 * A reference counted class, can be used as alternate base
 * of auto_handle instead of the default base_handle (and
 * this turn auto_handle into a reference-counted resource wrapper)
 */
template<
    typename H,
    typename T = handle_traits<H>,
    typename R = unsigned int>
class ref_counted_base_handle
{
public:
    typedef R counter_type;

protected:
    // destructor, deletes the counter if this
    // is the last instance
    ~ref_counted_base_handle()
    {
        if (ref_)
        {
            assert(*ref_ != 0);
            if (--*ref_ == 0)
            {
                delete ref_;
            }
        }
    }

    explicit ref_counted_base_handle(H h) : ref_(0)
    {
        if (h != T::null_value())
        {
            ref_ = new counter_type(1);
        }
    }

    // Constructor with extra parameter;
    // allows the counter to be tied to
    // another object.
    template<typename U>
    ref_counted_base_handle(H h, U u) : ref_(0)
    {
        if (h != T::null_value())
        {
            ref_ = new counter_type(1, u);
        }
    }

    // copy construction
    ref_counted_base_handle(const ref_counted_base_handle& other)
        : ref_(other.ref_)
    {
        if (ref_)
            ++*ref_; // bump up the ref count
    }

    template<typename U, typename V, typename X>
    ref_counted_base_handle(const ref_counted_base_handle<U, V, X>& other)
        : ref_(other.ref_)
    {
        if (ref_)
            ++*ref_; // bump up the ref count
    }

    // assignment operator
    ref_counted_base_handle& operator=(const ref_counted_base_handle& other)
    {
        ref_counted_base_handle tmp(other);
        tmp.swap(*this);
        return *this;
    }

public:
    void swap(ref_counted_base_handle& that) throw()
    { std::swap(ref_, that.ref_); }

    // returns the reference counter value
    const counter_type& count() const
    // counter_type count() const
    {
        static const counter_type zero = counter_type();
        if (ref_) return *ref_;
        // return counter_type();
        return zero;
    }

protected:
    void dispose(H h) throw()
    {
        if (count() == 1)
        {
            T::dispose(h);
        }
    }

    // place-holder, required
    // by auto_handle
    static H copy(H h) throw() { return h; }

#ifdef REFCOUNTED_AUTO_HANDLE_CAN_RELEASE_OWNERSHIP
    H release(H& h) throw()
    {
        H res = h;
        if (res != T::null_value())
        {
            if (count() == 1)
            {
                delete ref_;
                ref_ = 0;
                h = T::null_value();
            }
            else
                // not safe to release ownership,
                // there are other references to it
                return T::null_value();

        }
        return res;
    }
#else // simply prevent compilation
    struct CANNOT_RELEASE_OWNERSHIP_SAFELY;
    static H release(H&)
    { return sizeof(CANNOT_RELEASE_OWNERSHIP_SAFELY); }
#endif

private:
    template<typename U, typename V, typename X>
    friend class ref_counted_base_handle;

    counter_type* ref_;
};

#endif // REF_COUNTED_BASE_HANDLE_H__1060467395
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
