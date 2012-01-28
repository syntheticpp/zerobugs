#ifndef AUTO_HANDLE_H__1028358189
#define AUTO_HANDLE_H__1028358189
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
//
// From my C/C++ Users Journal (Aug 2001 issue) article
//  "Generalizing The Concept of auto_ptr"
//
// Comments such as "20.4.5.1 construct/copy/destroy"
// refer to the C++ standard paragraph that specifies
// an auto_ptr behavior that auto_handle mimics.
//
#include <algorithm>
#include "generic/export.h"


template<typename H>
struct handle_traits
{
    static H null_value();
    static void dispose(H) throw();
    static H clone(H h);
    static void init(H);
};

// default auto_handle base class:
// copy ctor is disabled
template<class H, class T>
class base_handle
{
public:
    explicit base_handle(H) {}

protected:
    static void dispose(H h) throw()
    { T::dispose(h); }

    static H copy(H h) throw()
    { return h; }

    // Place-holder. This is needed in the case
    // where a class that maintains some state
    // is used instead of base_handle.
    static void swap(base_handle&) throw() {}

    static H release(H& h) throw()
    {
        H res(h);
        h = T::null_value();
        return res;
    }

private:
    // Default behavior is non-copyable.
    // non-const arg on purpose!
    base_handle(base_handle&);
    base_handle& operator=(base_handle&);
};

// helper for returning
// auto_handles by value
/* template<class H>
struct auto_handle_ref
{
    explicit auto_handle_ref(H h) : handle_(h) {}
    H handle_;
}; */

// auto_handle
template<
    typename H,
    typename T = handle_traits<H>,
    typename B = base_handle<H, T> >
class auto_handle : public B
{
public:
    typedef H handle_type;
    typedef T traits;
    typedef H (auto_handle::*unspecified_bool_type)() const;

    // 20.4.5.1 construct/copy/destroy
    explicit auto_handle(
        H h = T::null_value()) throw()
    : B(h), handle_(h)
    {}

    // copy ctor
    auto_handle(const auto_handle& that)
    // auto_handle(auto_handle& that)
        : B(that)
        , handle_(B::copy(that.handle_))
    {}

    // template copy ctor -- to support
    // implicit conversions (such as from
    // derived to base);
    // takes non-const reference
    // as parameter to support exclusive ownership
    // policies
    template<typename U, typename V, typename X>
    auto_handle(auto_handle<U, V, X>& other)
        : B(other)
        , handle_(B::copy(other.handle_))
    {}

    /* template<typename U, typename V, typename X>
    auto_handle(const auto_handle<U, V, X>& other)
        : B(other)
        , handle_(B::copy(other.handle_))
    {} */

    template<typename U>
    auto_handle(H h, U u) : B(h, u), handle_(h)
    {}

    ~auto_handle() throw()
    {
        B::dispose(handle_);
    }

    // 20.4.5.2 members
    H get() const throw()
    // Wed Jan 23  2008: I do not recall why this is commented out
    //{ return handle_ == T::null_value() ? H() : handle_; }
    { return handle_; }

    // release ownership
    H release() throw()
    { return B::release(handle_); }

    void reset(H handle = T::null_value()) throw()
    {
        auto_handle tmp(handle);
        tmp.swap(*this);
    }
    operator unspecified_bool_type() const
    {
        return this->get() == 0 ? 0 : &auto_handle::get;
    }
/*
    // 20.4.5.3 conversions
    // implicit ctor, clients may write
    // auto_handle<some_class> h = func()
    // where func returns auto_handle by
    // value
    auto_handle(
        const auto_handle_ref<H>& r) throw()
        : B(r.handle_), handle_(r.handle_)
    {}

    // other operators
    auto_handle& operator=(
        const auto_handle_ref<H>& r)
    {
        auto_handle tmp(r);
        this->swap(tmp);
        return *this;
    }

    operator auto_handle_ref<H>()
    { return auto_handle_ref<H>(release()); }
 */
    auto_handle& operator=(const auto_handle& rhs)
    {
        auto_handle tmp(rhs);
        this->swap(tmp);
        return *this;
    }

    bool is_valid() const
    { return handle_ != T::null_value(); }

    bool operator !() const
    { return !this->is_valid(); }

    void swap(auto_handle& that) throw()
    {
        B::swap(that);
        std::swap(handle_, that.handle_);
    }

protected:
    // for upcasting in derived classes
    void set(H& handle) { handle_ = handle; }

private:
    template<typename U, typename V, typename X>
    friend class auto_handle;

    H handle_;
}; // auto_handle


// non-clonable array traits
template<typename T>
struct array_traits
{
    static T null_value()
    { return 0; }

    static void dispose(T t) throw()
    { delete[] t; }
};

#endif // AUTO_HANDLE_H__1028358189
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
