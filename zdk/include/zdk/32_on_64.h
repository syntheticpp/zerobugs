#ifndef _32_ON_64_H__851CB8B6_FCAF_4055_901A_E74F21C51485
#define _32_ON_64_H__851CB8B6_FCAF_4055_901A_E74F21C51485
//
// $Id$
//
// Utilities for cross-debugging 32 bit apps on 64 bit hosts.
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include "zdk/export.h"


struct Thread;

namespace Platform
{
    namespace detail
    {
        template<bool>
        struct ZDK_LOCAL ApplyMask
            {
                template<typename T>
                void operator()(T& value) const
                {
                    value &= 0xffffffff;
                }
            };
        template<>
            struct ZDK_LOCAL ApplyMask<false>
            {
                template<typename T> void operator()(T&) const { }
            };
    }
    template<typename T>
       inline void ZDK_LOCAL after_read(const Thread& thread, T& val)
       {
            if ((__WORDSIZE != 32) && thread.is_32_bit())
            {
                detail::ApplyMask<boost::is_integral<T>::value>()(val);
            }
       }

   template<typename T>
       inline void ZDK_LOCAL inc_word_ptr(const Thread& thread, T& wp, int step = 1)
       {
            if ((__WORDSIZE != 32) && thread.is_32_bit())
            {
                wp += 4 * step;
            }
            else
            {
                wp += sizeof(long) * step;
            }
       }

   template<typename T>
       inline void ZDK_LOCAL dec_word_ptr(const Thread& thread, T& wp, int step = 1)
       {
            if ((__WORDSIZE != 32) && thread.is_32_bit())
            {
                wp -= 4 * step;
            }
            else
            {
                wp -= sizeof(long) * step;
            }
       }
}
#endif // _32_ON_64_H__851CB8B6_FCAF_4055_901A_E74F21C51485
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
