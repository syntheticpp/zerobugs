#ifndef _TYPE_SYSTEM_UTIL_H_
#define _TYPE_SYSTEM_UTIL_H_
//
// $Id$
//
// Convenience functions for working with the TypeSystem
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <algorithm>
#include <functional>
#include <vector>
#include <boost/limits.hpp>
#include "zdk/get_pointer.h"
#include "zdk/shared_string_impl.h"
#include "zdk/type_system.h"


inline FunType* ZDK_LOCAL
get_function_type(TypeSystem& types,
                   const RefPtr<DataType>& retType,
                   const std::vector<DataType*>& paramTypes,
                   bool varArgs)
{
    return types.get_fun_type(retType.get(),
                              &paramTypes[0],
                              paramTypes.size(),
                              varArgs);
}



template<typename V>
inline FunType* ZDK_LOCAL
get_function_type(TypeSystem& types,
                  const RefPtr<DataType>& retType,
                  const V& paramTypes,
                  bool varArgs)
{
    std::vector<DataType*> tmp(paramTypes.size());
    for (size_t i = 0; i != paramTypes.size(); ++i)
    {
        tmp[i] = get_pointer(paramTypes[i]);
    }
    return types.get_fun_type(retType.get(), &tmp[0], tmp.size(), varArgs);
}



template<typename T>
inline DataType* ZDK_LOCAL
get_int_type(TypeSystem& types, T*, const char* name)
{
    assert(std::numeric_limits<T>::is_integer);
    return types.get_int_type(shared_string(name).get(),
                        sizeof(T) * Platform::byte_size,
                        std::numeric_limits<T>::is_signed);
}



template<typename T>
inline DataType* ZDK_LOCAL
get_float_type(TypeSystem& types, T*, const char* name)
{
    assert(!std::numeric_limits<T>::is_exact);
    return types.get_float_type(shared_string(name).get(), sizeof(T));
}


#define GET_INT_TYPE(types,t) ::get_int_type(types, (t*)0,#t)
#define GET_FLOAT_TYPE(ts,t) ::get_float_type(ts, (t*)0, #t)

#endif //_TYPE_SYSTEM_UTIL_H_
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
