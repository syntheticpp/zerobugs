#ifndef SINGLETON_H__1040499294
#define SINGLETON_H__1040499294
//
// $Id: singleton.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "generic/export.h"

/**
 * Instantiates the single instance of a class
 */
class ZDK_LOCAL DefaultSingletonFactory
{
public:
    typedef DefaultSingletonFactory type;

    template<typename T>
    static T& instance(T* /* dummy, just to pass in the type */)
    //{ static T inst; return inst; }
    { static T* inst = new T(); return *inst; }
};


#define S(x) #x
#define STR(x) S(x)



template<typename T, typename F = DefaultSingletonFactory>
class ZDK_LOCAL Singleton : public T
{
    typedef F factory_type;
#ifdef __INTEL_COMPILER
    friend class F;
#elif (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4 && __GNUC_PATCHLEVEL__ < 5))
    friend typename F::type;
#else
    friend class F::type;
#endif

public:
    static Singleton& instance()
    {
        return factory_type::instance(static_cast<Singleton*>(0));
    }

    static const Singleton& const_instance()
    { return instance(); }

private:
    Singleton() {}
    ~Singleton() throw() {}

private:
    Singleton(const Singleton&);
    Singleton& operator=(const Singleton&);
};

#endif // SINGLETON_H__1040499294
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
