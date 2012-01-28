#ifndef VISITOR_H__DF4B67ED_92E8_4AF3_A238_91FA1E3C6CC6
#define VISITOR_H__DF4B67ED_92E8_4AF3_A238_91FA1E3C6CC6
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
// Adapted from Andrei Alexandrescu's book:
//  "Modern C++ Design", Addison & Wesley 2001
//
#include <cassert>
#include <functional>
#include <boost/utility.hpp>
#include "generic/export.h"

#ifdef DEBUG
 #include <iostream>
 #include <typeinfo>
#endif // DEBUG


class BaseVisitor
{
protected:
    BaseVisitor() {}
    virtual ~BaseVisitor() {}
};


template<class T, typename R = void>
struct BasicVisitor
{
    typedef R return_type;

#if (__GNUC__ >= 3)
// g++ 2.95 requires the __attribute__((com_interface))
// hack, which is not compatible with virtual dtors.
    virtual ~BasicVisitor() { }
#endif
};


template<class T, typename R = void>
struct Visitor : public BasicVisitor<T, R>
{
    virtual R visit(T&) = 0;
};
template<class T, typename R = void>
struct ConstVisitor : public BasicVisitor<T, R>
{
public:
    virtual R visit(const T&) = 0;
};



template<typename R = void>
class BaseVisitable
{
public:
    typedef R return_type;

protected:
    BaseVisitable() { }
    virtual ~BaseVisitable() { }

    template<class T>
    static R default_accept_impl(const T& host, BaseVisitor& guest)
    {
    #ifdef DEBUG
        std::clog << "BaseVisitable::accept_impl(";

    #if (__GNUC__ >= 2)
        // com_interface conflicts with typeid in gcc 2.95
        std::clog << typeid(host).name() << ", ";
    #endif
        std::clog << typeid(guest).name() << ")\n";

        const static bool no_unexpected_type(false);
        assert(no_unexpected_type);
    #endif // DEBUG

        return R();
    }
};


template<typename R = void>
class Visitable : public BaseVisitable<R>
{
public:
    virtual R accept(BaseVisitor*) = 0;

protected:
    // acyclic visitor
    template<class T>
    static R accept_impl(T& host, BaseVisitor& guest)
    {
        if (Visitor<T, R>* p = dynamic_cast<Visitor<T, R>*>(&guest))
        {
            return p->visit(host);
        }
        return default_accept_impl(host, guest);
    }
};


template<typename R = void>
class ConstVisitable : public BaseVisitable<R>
{
public:
    virtual R accept(BaseVisitor*) const = 0;

protected:
    template<class T>
    static R accept_impl(const T& host, BaseVisitor& guest)
    {
        if (ConstVisitor<T, R>* p = dynamic_cast<ConstVisitor<T, R>*>(&guest))
        {
            return p->visit(host);
        }
        return default_accept_impl(host, guest);
    }
};


#define DECLARE_VISITABLE() \
    virtual return_type accept(BaseVisitor* guest) \
    { assert(guest); return accept_impl(*this, *guest); }

#define DECLARE_CONST_VISITABLE() \
    virtual return_type accept(BaseVisitor* guest) const \
    { assert(guest); return accept_impl(*this, *guest); }


/**
 * Functor used with for_each on collections of
 * pointers to visitable objects
 */
template<typename R = void>
class Accept : public std::unary_function<BaseVisitable<R>*, R>
{
public:
    explicit Accept(BaseVisitor& v) : v_(v)
    {}

    R operator()(const BaseVisitable<R>* ptr)
    { return ptr->accept(&v_); }

private:
    BaseVisitor& v_;
};

#endif // VISITOR_H__DF4B67ED_92E8_4AF3_A238_91FA1E3C6CC6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
