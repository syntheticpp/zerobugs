#ifndef ZOBJECT_IMPL_H__525D708D_534F_41CC_B2C8_E20A8A6B95B3
#define ZOBJECT_IMPL_H__525D708D_534F_41CC_B2C8_E20A8A6B95B3
//
// $Id: zobject_impl.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/type_traits/is_same.hpp>
#include "zdk/export.h"
#include "zdk/ref_counted_impl.h"
#include "zdk/zobject.h"


namespace detail
{
    template<bool, typename T>
    struct ZDK_LOCAL RefCountedBaseHelper
    {
        struct Base : public T, public RefCountedImpl<>
        {
            virtual ~Base() throw() {}
        };
    };

    template<typename T>
    struct ZDK_LOCAL RefCountedBaseHelper<true, T>
    {
        struct Base : public RefCountedImpl<T>
        {
            virtual ~Base() throw() {}
        };
    };

} // detail


template<typename T> class ZDK_LOCAL RefCountedMetaBase
{
    enum
    {
        flag = boost::is_base_and_derived<RefCounted, T>::value
            || boost::is_same<RefCounted, T>::value
    };

public:
    typedef typename detail::RefCountedBaseHelper<flag, T>::Base type;
};


template<typename T = ZObject>
struct ZDK_LOCAL ZObjectImpl : public RefCountedMetaBase<T>::type
{
BEGIN_INTERFACE_MAP(ZObjectImpl<T>)
    INTERFACE_ENTRY(T)
END_INTERFACE_MAP()
};


#endif // ZOBJECT_IMPL_H__525D708D_534F_41CC_B2C8_E20A8A6B95B3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
