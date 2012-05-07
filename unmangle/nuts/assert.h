#ifndef ASSERT_H__261A2F0A_B1BC_4C90_9875_15426B1A7217
#define ASSERT_H__261A2F0A_B1BC_4C90_9875_15426B1A7217
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

#if (__GNUC__ >= 4) && !defined(__INTEL_COMPILER)
 #pragma GCC visibility push(default)
#endif


#include <cassert>
#include <string>
#include <stdexcept>


#if 0
 #define NUTS_ASSERT assert
#else
 #define __STR(x) #x
 #define TO_STRING(x) __STR(x)

 #define NUTS_ASSERT(e) while(!(e)) { \
    throw std::runtime_error(__FILE__ ":" TO_STRING(__LINE__)": Assertion failed: " # e); }
#endif

#if DEBUG
 #define NUTS_EXHAUSTED_FIXED_MEM() exhausted_fixed_mem(__func__)
#else
 #define NUTS_EXHAUSTED_FIXED_MEM() // as nothing
#endif

namespace nuts
{
    static void inline exhausted_fixed_mem(const std::string& func)
    {
        throw std::runtime_error(func + ": fixed memory space exhausted");
    }
}

#if (__GNUC__ >= 4) && !defined(__INTEL_COMPILER)
 #pragma GCC visibility pop
#endif
#endif // ASSERT_H__261A2F0A_B1BC_4C90_9875_15426B1A7217
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
