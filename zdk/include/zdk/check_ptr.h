#ifndef CHECK_PTR_H__CC3C0040_A150_46CA_BC94_58DA32DC7469
#define CHECK_PTR_H__CC3C0040_A150_46CA_BC94_58DA32DC7469
//
// $Id: check_ptr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <stdexcept>
#include "zdk/get_pointer.h"


#if !defined(NDEBUG) || (__GNUC__ < 3)
 #define CHKPTR(x) check_ptr(x, __FILE__, __LINE__)
#else
 #define CHKPTR(x) check_ptr(x)
#endif


template<typename T> struct check_ptr_traits
{
    static void validate(T) {}
};
/*
template<>
struct check_ptr_traits<RefCounted*>
{
    static void validate(RefCounted* ptr)
    {
    }
};
*/

template<typename T>
inline ZDK_LOCAL T check_ptr(T ptr, const char* file, int line)
{
    if (!get_pointer(ptr))
    {
    #ifdef DEBUG
        fprintf(stderr, "%s line %d: ptr != NULL\n", file, line);
        abort();
    #else
        std::ostringstream msg;
        msg << file << ':' << line << ": null pointer";
        throw std::invalid_argument(msg.str());
    #endif
    }
    check_ptr_traits<T>::validate(ptr);
    return ptr;
}

template<typename T>
inline ZDK_LOCAL T check_ptr(T ptr)
{
    if (!get_pointer(ptr))
    {
        // set this environment variable to generate a coredump
        static bool doAbort = getenv("ZERO_NULL_PTR_ABORT");
        if (doAbort)
        {
            abort();
        }
        throw std::invalid_argument("null pointer");
    }
    check_ptr_traits<T>::validate(ptr);
    return ptr;
}

template<typename T>
inline T* check_ptr(const std::auto_ptr<T>& ptr)
{
    return check_ptr(ptr.get());
}

template<typename T>
inline T* check_ptr(const std::auto_ptr<T>& ptr, const char* file, int line)
{
    return check_ptr(ptr.get(), file, line);
}


#endif // CHECK_PTR_H__CC3C0040_A150_46CA_BC94_58DA32DC7469
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
