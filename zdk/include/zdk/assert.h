#ifndef ASSERT_H__30549794_7F62_470B_9B45_AD1D0075D381
#define ASSERT_H__30549794_7F62_470B_9B45_AD1D0075D381
//
// $Id: assert.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include "zdk/export.h"
#include "zdk/stdexcept.h"

#include <assert.h>
#if (__GNUC__ == 2)
//
// gcc 2.95 crashes while attempting to compile this with -O2
//
 #define assert_gt(x,y)  assert((x) > (y))
 #define assert_eq(x,y)  assert((x)==(y))
 #define assert_gte(x,y) assert((x) >= (y))
#else

 #include <limits>
 #include <boost/format.hpp>


class assert_error : public std::logic_error
{
    inline static std::string format(const char* file, size_t line)
    {
        return (boost::format(
            "Assertion failed at: %1%:%2%") % file % line).str();
    }
public:
    assert_error(const char* file, size_t line)
        : std::logic_error(format(file, line))
    { }

    template<typename T>
    assert_error(const T& msg, const char* file, size_t line)
        : std::logic_error(msg + format(file, line))
    { }
};



// Assert functions that avoid the signed vs. unsigned
// compiler warnings. Also, throw an assert_error exception
// rather than aborting on failure
namespace detail
{
    template<typename T, typename U, int>
    struct numeric_assert
    {
        static bool eq(T lhs, U rhs) { return(lhs == rhs); }
        static bool gt(T lhs, U rhs) { return(lhs > rhs); }
        //static bool gte(T lhs, U rhs) { return(lhs >= rhs); }
    };
    template<typename T, typename U>
    struct numeric_assert<T, U, 1>
    {
        static bool gt(T lhs, U rhs)
        { return ((lhs >= 0) && (static_cast<U>(lhs) > rhs)); }

        static bool eq(T lhs, U rhs)
        { return ((rhs >= 0) && (rhs == static_cast<U>(lhs))); }

        //static bool gte(T lhs, U rhs)
        //{ return ((lhs >= 0) && (static_cast<U>(lhs) >= rhs)); }
    };
    template<typename T, typename U>
    struct numeric_assert<T, U, -1>
    {
        static bool gt(T lhs, U rhs)
        { return ((rhs < 0) || (lhs > static_cast<T>(rhs))); }

        static bool eq(T lhs, U rhs)
        { return ((rhs >= 0) && (lhs == static_cast<T>(rhs))); }

        static bool gte(T lhs, U rhs)
        { return ((rhs < 0) || (lhs >= static_cast<T>(rhs))); }
    };
}

template<typename T, typename U>
void inline ZDK_LOCAL __assert_gt(T lhs, U rhs, const char* file, size_t line)
{
    typedef detail::numeric_assert<T, U,
        std::numeric_limits<T>::is_signed - std::numeric_limits<U>::is_signed> Assert;
    if (!Assert::gt(lhs, rhs))
    {
        throw assert_error((boost::format("%1% > %2%: ") % lhs % rhs).str(), file, line);
    }
}

template<typename T, typename U>
void inline ZDK_LOCAL __assert_gte(T lhs, U rhs, const char* file, size_t line)
{
    typedef detail::numeric_assert<T, U,
        std::numeric_limits<T>::is_signed - std::numeric_limits<U>::is_signed> Assert;

    if (!Assert::gte(lhs, rhs))
    {
#ifdef DEBUG
        throw assert_error((boost::format("%1% >= %2%: ") % lhs % rhs).str(),
                file, line);
#else
        throw assert_error(file, line);
#endif
    }
}

template<typename T, typename U>
void inline ZDK_LOCAL __assert_eq(T lhs, U rhs, const char* file, size_t line)
{
    typedef detail::numeric_assert<T, U,
        std::numeric_limits<T>::is_signed -
            std::numeric_limits<U>::is_signed> Assert;
    if (!Assert::eq(lhs, rhs))
    {
#ifdef DEBUG
        throw assert_error((boost::format("%1% == %2%: ") % lhs % rhs).str(),
                file, line);
#else
        throw assert_error(file, line);
#endif
    }
}

#ifdef NDEBUG
 #define assert_gte(x,y)
 #define assert_gt(x,y)
 #define assert_eq(x,y)
#else
 #define assert_gt(x,y) __assert_gt((x), (y), __FILE__, __LINE__)
 #define assert_gte(x,y) __assert_gte((x), (y), __FILE__, __LINE__)
 #define assert_eq(x,y) __assert_eq((x), (y), __FILE__, __LINE__)
#endif // NDEBUG

#endif // __GNUC__ > 2

#endif // ASSERT_H__30549794_7F62_470B_9B45_AD1D0075D381
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
