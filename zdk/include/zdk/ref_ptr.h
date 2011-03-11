#ifndef REF_PTR_H__1060290108
#define REF_PTR_H__1060290108
//
// $Id: ref_ptr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/interface_cast.h"
#include "zdk/ref_counted.h"


/**
 * Smart pointer: wraps objects that support
 * intrusive reference counted (such as RefCounted).
 */
template<typename T>
class ZDK_LOCAL RefPtr
{
    T* ptr_;

    RefCounted* base_ptr() { return ptr_; }

    void inc_ref()
    {
        if (ptr_)
        {
            base_ptr()->inc_ref();
        }
    }

public:
    typedef T element_type;
    typedef T value_type;
    typedef T* pointer;
    typedef T* (RefPtr::*unspecified_bool_type)() const volatile;

    template<typename U> friend class RefPtr;

    RefPtr(T* ptr = 0) : ptr_(ptr)
    {
        inc_ref();
    }

    /**
     * Generalized copy ctor
     */
    template<typename U>
    RefPtr(const RefPtr<U>& other) : ptr_(other.ptr_)
    {
        inc_ref();
    }
/* -VS 2010 doesn't like this
    RefPtr(const RefPtr& other) : ptr_(other.ptr_)
    {
        inc_ref();
    }
*/
/*
    RefPtr(RefPtr&& other) : ptr_(other.ptr_)
    {
        other.ptr_ = 0;
    }
 */
    RefPtr(const volatile RefPtr& other) : ptr_(other.ptr_)
    {
        inc_ref();
    }
    ~RefPtr() throw()
    {
        if (ptr_)
        {
            base_ptr()->release();
        }
    }

    operator unspecified_bool_type() const volatile
    {
        return ptr_ == 0 ? 0 : &RefPtr::get;
    }

    bool is_null() const volatile
    {
        return ptr_ == 0;
    }

    void reset(T* ptr = NULL)
    {
        RefPtr tmp(ptr);
        tmp.swap(*this);
    }

    RefPtr& operator=(const RefPtr& that)
    {
        RefPtr tmp(that);
        this->swap(tmp);
        return *this;
    }
   /* 
    RefPtr& operator=(RefPtr&& that)
    {
        RefPtr tmp(std::forward<RefPtr>(that));
        this->swap(tmp);
        return *this;
    } */

    template<typename U>
    RefPtr& operator=(const RefPtr<U>& that)
    {
        RefPtr tmp(that);
        this->swap(tmp);
        return *this;
    }

    template<typename U>
    RefPtr& operator=(U* ptr)
    {
        RefPtr tmp(ptr);
        this->swap(tmp);
        return *this;
    }
    void swap(RefPtr& other) throw()
    {
        ::std::swap(ptr_, other.ptr_);
    }

    T* operator->() const volatile throw()
    {
        assert(ptr_);
        return ptr_;
    }

    T* get() const volatile throw()
    {
        return ptr_;
    }

    T& operator*() const
    {
        assert(ptr_);
        return *ptr_;
    }

    T* detach()
    {
        T* temp = ptr_;
        if (temp)
        {
            base_ptr()->dec_ref_and_test();
            ptr_ = 0;
        }
        return temp;
    }
};


template<typename T, typename U>
bool
inline ZDK_LOCAL operator<(const RefPtr<T>& lhs, const RefPtr<U>& rhs)
{
    return static_cast<T*>(lhs.get()) < static_cast<U*>(rhs.get());
}


template<typename T, typename U>
bool
inline ZDK_LOCAL operator==(const RefPtr<T>& lhs, const RefPtr<U>& rhs)
{
    return static_cast<T*>(lhs.get()) == static_cast<U*>(rhs.get());
}


#if defined(STLPORT) || (__GNUC__ >= 3)
template<typename T, typename U>
bool
inline ZDK_LOCAL operator!=(const RefPtr<T>& lhs, const RefPtr<U>& rhs)
{
    return !(lhs == rhs);
}
#endif


template<typename T, typename U>
RefPtr<T>
inline ZDK_LOCAL interface_cast(const RefPtr<U>& u)
{
    T* ptr = 0;

    if (u.get())
    {
        if (u->query_interface(T::_uuid(), (void**)&ptr))
        {
            assert(ptr);
        }
    }
    return ptr;
}

template<typename T>
inline bool ZDK_LOCAL
query_interface_aggregate(uuidref_t iid, const RefPtr<T>& ptr, void** p)
{
    if (ptr)
    {
        if (uuid_equal(iid, ptr->_uuid()))
        {
            *p = ptr.get();
            return true;
        }
    }
    return false;
}

#endif // REF_PTR_H__1060290108
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
