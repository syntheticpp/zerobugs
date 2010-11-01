#ifndef PLATFORM_H__4AB143C2_AE6A_4F26_95A6_FCFB88D162B4
#define PLATFORM_H__4AB143C2_AE6A_4F26_95A6_FCFB88D162B4
//
// $Id: platform.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/cstdint.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include "zdk/config.h"

using boost::uint32_t;
using boost::uint64_t;
using boost::int32_t;
using boost::int64_t;


namespace Platform
{
    typedef long word_t; // machine-word
    typedef size_t bitsize_t;

    enum { byte_size = std::numeric_limits<unsigned char>::digits };
    enum { long_size = std::numeric_limits<unsigned long>::digits };

#if defined(__i386__)
    BOOST_STATIC_ASSERT(byte_size == 8);
    BOOST_STATIC_ASSERT(long_size == 32);
    typedef uint32_t addr_t;
    typedef uint32_t reg_t;

#elif defined(__x86_64__)
    BOOST_STATIC_ASSERT(byte_size == 8);
    BOOST_STATIC_ASSERT(long_size == 64);
    typedef uint64_t addr_t;
    typedef uint64_t reg_t;

#else
    // generic platform
    typedef unsigned long addr_t;
    typedef unsigned long reg_t;
#endif

} // Platform

#endif // PLATFORM_H__4AB143C2_AE6A_4F26_95A6_FCFB88D162B4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
