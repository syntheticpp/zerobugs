#ifndef WEAK_PTR_H__8488FF5C_3D34_48F8_B0B7_991665306287
#define WEAK_PTR_H__8488FF5C_3D34_48F8_B0B7_991665306287
//
// $Id: weak_ptr.h 723 2010-10-28 07:40:45Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <algorithm>            // std::swap
#include "zdk/ref_ptr.h"
#include "zdk/weak_ref_impl.h"


template<typename T>
class ZDK_LOCAL WeakPtr
{
    WeakRef* ref_;

    T* get_ptr() const volatile throw()
    {
        ZDK_BARRIER();
        if (ref_)
        {
            assert(ref_->count());

            T* p = static_cast<T*>(ref_->get_ptr());
            return p;
        }
        return NULL;
    }

public:
    template<typename U, typename V>
    friend bool operator<(const WeakPtr<U>&, const WeakPtr<V>&);

    typedef T element_type;
    typedef T value_type;
    typedef T* pointer;

    ~WeakPtr() throw()
    {
        if (ref_)
        {
            ref_->release();
        }
    }

    WeakPtr(const T* ptr = NULL) : ref_(NULL)
    {
        if (ptr)
        {
            ref_ = static_cast<const RefCounted*>(ptr)->weak_ref();
            ref_->add_ref();
        }
    }

    WeakPtr(const WeakPtr& other) : ref_(other.ref_)
    {
        if (ref_)
        {
            ref_->add_ref();
        }
    }

    template<typename U>
    WeakPtr(const RefPtr<U>& ptr) : ref_(NULL)
    {
        if (ptr)
        {
            RefCounted* p = ptr.get();
            ref_ = p->weak_ref();
            ref_->add_ref();
        }
    }

    template<typename U>
    WeakPtr& operator=(const RefPtr<U>& ptr)
    {
        WeakPtr tmp(ptr);
        std::swap(ref_, tmp.ref_);
        return *this;
    }

    WeakPtr& operator=(const WeakPtr& other)
    {
        WeakPtr tmp(other);
        std::swap(ref_, tmp.ref_);
        return *this;
    }

    RefPtr<T> ref_ptr() const volatile throw()
    {
        RefPtr<T> ptr(this->get_ptr());
        if (ptr)
        {
            RefCounted* rc = ptr.get();

            // make sure the counter does not go negative
            // when RefPtr<> is done with
            assert(rc->ref_count() >= 2);

            // WeakRefImpl::get_ptr() bumps up the ref count
            rc->dec_ref_and_test();
        }
        return ptr;
    }
    /// for compat with boost::weak_ptr
    RefPtr<T> lock() const volatile throw() { return ref_ptr(); }

    RefPtr<T> operator->() const
    {
        RefPtr<T> ptr = ref_ptr();
        assert(ptr);
        return ptr;
    }

    long ref_count() const volatile
    {
        return ref_ ? ref_->count() : 0;
    }

    void reset() { ref_ = 0; }
};


template<typename T, typename U>
bool
inline ZDK_LOCAL operator<(const WeakPtr<T>& lhs, const WeakPtr<U>& rhs)
{
    return lhs.ref_ptr() < rhs.ref_ptr();
}


template<typename T, typename U>
bool
inline ZDK_LOCAL operator==(const WeakPtr<T>& lhs, const WeakPtr<U>& rhs)
{
    return lhs.ref_ptr() == rhs.ref_ptr();
}


template<typename T, typename U>
bool
inline ZDK_LOCAL operator!=(const WeakPtr<T>& lhs, const WeakPtr<U>& rhs)
{
    return !(lhs == rhs);
}


template<typename T, typename U>
RefPtr<T>
inline ZDK_LOCAL interface_cast(const WeakPtr<U>& u)
{
    if (RefPtr<U> up = u.ref_ptr())
    {
        return interface_cast<T, U>(up);
    }
    return NULL;
}
#endif // WEAK_PTR_H__8488FF5C_3D34_48F8_B0B7_991665306287
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
