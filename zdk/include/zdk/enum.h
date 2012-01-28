#ifndef ENUM_H__1060222855
#define ENUM_H__1060222855
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
#include <functional>
#include "zdk/ref_ptr.h"
#include "zdk/unknown2.h"


/**
 * There are cases when a list of objects is needed, but cannot
 * be returned in a standard container (because of interface rules)
 * To simplify memory management the enumerate() methods take
 * a pointer to an implementation of EnumCallback<>, and call
 * its notify() method for each element.
 */
template<typename T, typename R = void>
DECLARE_ZDK_INTERFACE(EnumCallback)
{
    virtual R notify(T) = 0;

    /**
     * @return number of elements enumerated so far, -1 if not known
     */
    virtual long _count() const { return -1; }
};


template<typename T, typename V, typename R = void>
DECLARE_ZDK_INTERFACE(EnumCallback2)
{
    virtual R notify(T, V) = 0;
};


template<typename T, typename U, typename V, typename R = void>
DECLARE_ZDK_INTERFACE(EnumCallback3)
{
    virtual R notify(T, U, V) = 0;
};


///// Helpers /////
/**
 * Predicate for calling the EnumCallback interface
 */
template<typename T>
class ZDK_LOCAL EnumCallbackCaller : public std::unary_function<T*, void>
{
public:
    typedef EnumCallback<T*> Callback;

    explicit EnumCallbackCaller(Callback& cb) : callback_(cb)
    {
    }

    void operator()(T* ptr) const
    {
        callback_.notify(ptr);
    }

    void operator()(const RefPtr<T>& ptr) const
    {
        callback_.notify(ptr.get());
    }

private:
    Callback& callback_;
};

/* Convenience function, lets the compiler figure out
 * the actual template parameter type
 */
template<typename T> inline ZDK_LOCAL
EnumCallbackCaller<T> call_enum_callback(EnumCallback<T*>& callback)
{
    return EnumCallbackCaller<T>(callback);
}
#endif // ENUM_H__1060222855
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
