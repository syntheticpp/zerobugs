#ifndef INTERFACE_CAST_H__96D028A3_3959_4B41_967D_5D520D0A4965
#define INTERFACE_CAST_H__96D028A3_3959_4B41_967D_5D520D0A4965
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

#include "zdk/stdexcept.h"
#include "zdk/unknown2.h"
#include <cassert>
#include <string>
#include <typeinfo>
#include <boost/type_traits.hpp>



#define BAD_CAST(x) std::string("bad interface cast: ") + x::_type()


class ZDK_EXPORT bad_interface_cast : public std::exception
{
public:
    explicit bad_interface_cast(const std::string& type)
        : what_(type)
    { }

    virtual ~bad_interface_cast() throw() {}

    const char* what() const throw() { return what_.c_str(); }

private:
    std::string what_;
};


template<typename T>
inline ZDK_LOCAL void** void_cast(T* ptr)
{
    union
    {
        T* ptr;
        void** pv;
    } un;

    un.ptr = ptr;
    return un.pv;
}


template<typename T>
T inline ZDK_LOCAL interface_cast(Unknown2& u)
{
    typedef typename boost::remove_reference<T>::type V;
    V* ptr = 0;

    if (!u.query_interface(V::_uuid(), void_cast(&ptr)))
    {
        throw bad_interface_cast(BAD_CAST(V));
    }
    assert(ptr);
    return *ptr;
}


template<typename T>
T inline ZDK_LOCAL interface_cast(const Unknown2& u)
{
    typedef typename boost::remove_reference<T>::type U;
    typedef typename boost::remove_const<U>::type V;

    const V* ptr = 0;

    register Unknown2& x = const_cast<Unknown2&>(u);
    if (!x.query_interface(V::_uuid(), void_cast(&ptr)))
    {
        throw bad_interface_cast(BAD_CAST(V));
    }
    assert(ptr);
    return *ptr;
}


template<typename T>
T inline ZDK_LOCAL interface_cast(Unknown2* u)
{
    typedef typename boost::remove_pointer<T>::type V;
    T ptr = 0;

    if (u)
    {
        if (u->query_interface(V::_uuid(), void_cast(&ptr)))
        {
            assert(ptr);
        }
    }
    return ptr;
}


template<typename T>
T inline ZDK_LOCAL interface_cast(const Unknown2* u)
{
    typedef typename boost::remove_pointer<T>::type U;
    typedef typename boost::remove_const<U>::type V;

    T ptr = 0;

    if (u)
    {
        register Unknown2* x = const_cast<Unknown2*>(u);
        if (x->query_interface(V::_uuid(), void_cast(&ptr)))
        {
            assert(ptr);
        }
    }
    return ptr;
}
#undef BAD_CAST

#endif // INTERFACE_CAST_H__96D028A3_3959_4B41_967D_5D520D0A4965
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
