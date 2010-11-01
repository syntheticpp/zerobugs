#ifndef ALIGN_H__7C496F59_475F_41E4_B6A0_688601FB917B
#define ALIGN_H__7C496F59_475F_41E4_B6A0_688601FB917B
//
// $Id: align.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/platform.h"
#include "zdk/zero.h"

using Platform::addr_t;
using Platform::word_t;


template<typename T>
size_t inline ZDK_LOCAL round_to(size_t n)
{
    return (n + sizeof(T) - 1) / sizeof (T);
}

/**
 * round up to word size
 */
size_t inline ZDK_LOCAL round_to_word(size_t n)
{
    return round_to<word_t>(n);
}


/*
namespace detail
{
    template<typename T, bool> struct Align
    {
        static inline void apply(const Thread& t, addr_t& addr)
        {
            if (t.is_32_bit())
            {
                addr = round_to_word(addr) * sizeof(word_t);
            }
        }
    };

    //
    // do nothing
    //
    template<typename T> struct Align<T, false>
    {
        static inline void apply(const Thread&, T&) { }
    };
}


inline void word_align(const Thread& thread, addr_t& addr)
{
    //
    // apply to 64-bit machines, and up
    //
    detail::Align<addr_t, (sizeof(addr_t) > 4)>::apply(thread, addr);
}
*/

#endif // ALIGN_H__7C496F59_475F_41E4_B6A0_688601FB917B

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
