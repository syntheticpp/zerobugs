#ifndef TEST_H__DA2BA38F_64CA_496D_877D_909A37E42C55
#define TEST_H__DA2BA38F_64CA_496D_877D_909A37E42C55
//
//
// $Id$
//
// Simple unit test framework
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <exception>
#include <iostream>
#include <vector>

#if defined (__STDC__) || defined (__GNUC__)
 #define TEST_NAME(n)
 #define __test__ __func__
#else
 #define TEST_NAME(n) static const char __test__[] = #n;
#endif


#define BEGIN_TEST(func,param) void func param \
{   const size_t __attribute__((unused)) __line__ = __LINE__; \
    TEST_NAME(func) \
    std::cout << "--- " << __test__ << " --- \n"; \
    try \

#define END_TEST() \
    catch (std::exception& e) { \
        std::cout << __test__ << ": " << e.what() << std::endl; \
        assert(false); \
    } catch (...) { \
        std::cout << __test__ << ": unknown exception\n"; \
        assert(false); \
    } \
    std::cout << __test__ << ": OK.\n\n"; \
}

#ifdef NDEBUG
# undef NDEBUG
#endif
#include <cassert>

#endif // TEST_H__DA2BA38F_64CA_496D_877D_909A37E42C55
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
