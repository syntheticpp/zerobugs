#ifndef INT_TYPES_H__C195A481_D518_4632_A3DE_FD3492756542
#define INT_TYPES_H__C195A481_D518_4632_A3DE_FD3492756542
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
//
#include <cassert>
#include <cctype>
#include <boost/limits.hpp>
#include "generic/type_select.h"
#include "typez/public/type_tags.h"
#include "typez/public/value.h"


#define STRUCT  struct ZDK_LOCAL

/***************************************************************
 Helper templates for reading integer values of
 various size, with and without sign.
 ***************************************************************/
namespace detail
{
    template<int N> STRUCT int_type {};

    template<> STRUCT int_type<1>
    { typedef int8_t type; typedef uint8_t utype; };

    template<> STRUCT int_type<2>
    { typedef int8_t type; typedef uint8_t utype; };

    template<> STRUCT int_type<4>
    { typedef int8_t type; typedef uint8_t utype; };

    template<> STRUCT int_type<8>
    { typedef int8_t type; typedef uint8_t utype; };

    template<> STRUCT int_type<16>
    { typedef int16_t type; typedef uint16_t utype; };

    template<> STRUCT int_type<32>
    { typedef int32_t type; typedef uint32_t utype; };

    template<> STRUCT int_type<64>
    { typedef int64_t type; typedef uint64_t utype; };


    /**
     * Read integer type from string
     */
    template<typename T> STRUCT input_traits
    {
        static T from_string
        (
            const char* str,
            const char** p = 0,
            Unknown2* unk = 0
        )
        {
            T result = 0;
            if (!std::numeric_limits<T>::is_signed)
            {
                result = strtoul(str, const_cast<char**>(p), 0);
            }
            else
            {
                long x = strtol(str, const_cast<char**>(p), 0);
                if (*str != '-')
                {
                    unsigned long y = strtoul(str, 0, 0);
                    if (x ^ y)
                    {
                        x = y;
                    }
                }
                result = x;
            }

            put(unk, result);
            return result;
        }
    };

    /*
     * specializations for 64-bit integers
     */
    template<> STRUCT input_traits<int64_t>
    {
        static int64_t from_string
        (
            const char* str,
            const char** p = 0,
            Unknown2* unk = 0
        )
        {
            int64_t x = strtoll(str, const_cast<char**>(p), 0);
            if (*str != '-')
            {
                uint64_t y = strtoull(str, 0, 0);
                if (x ^ y)
                {
                    x = y;
                }
            }
            put(unk, x);
            return x;
        }
    };

    template<> STRUCT input_traits<uint64_t>
    {
        static uint64_t from_string
        (
            const char* str,
            const char** p = 0,
            Unknown2* unk = 0
        )
        {
            uint64_t x = strtoull(str, const_cast<char**>(p), 0);

            put(unk, x);
            return x;
        }
    };

    /*
     * Specializations for characters
     */
    template<> STRUCT input_traits<int8_t>
    {
        static int8_t from_string
        (
            const char* str,
            const char** p = 0,
            Unknown2* unk = 0
        )
        {
            int8_t result = 0;
            if (*str == '\'')
            {
                if (str[1] && str[2] == '\'' && str[3] == 0)
                {
                    if (p) *p = str + 3;
                    result = str[1];
                }
                else
                {
                    if (p) *p = str;
                }
            }
            else
            {
                result = strtol(str, const_cast<char**>(p), 0);
            }

            assert(sizeof result == 1);
            put(unk, result);

            return result;
        }
    };

    template<> STRUCT input_traits<uint8_t>
    {
        static uint8_t from_string
        (
            const char* str,
            const char** p = 0,
            Unknown2* unk = 0
        )
        {
            uint8_t result = input_traits<int8_t>::from_string(str, p);

            put(unk, result);
            return result;
        }
    };
} // detail namespace



template<typename T, int N>
STRUCT IntOutputHelper
{
    static const bool is_signed = std::numeric_limits<T>::is_signed;

    static void
    print(std::ostream& outs, T val, int nbits, size_t offs, bool isChar)
    {
        if (isChar)
        {
            if (nbits == 8
                && isprint(val & 0xff)
                && (outs.flags() & (std::ios::hex | std::ios::oct)) == 0)
            {
                outs << '\'' << char(val) << '\'';
                return;
            }
        }

#if (__BYTE_ORDER == __BIG_ENDIAN)
        offs = N - offs - nbits;
#endif
        if (offs)
        {
            val = val >> offs;
        }
        const T mask = T(1) << (nbits - 1);

        val &= ((mask - 1) << 1) | 1;

        if (is_signed && (outs.flags() & std::ios::hex) == 0)
        {
            if (val & mask)
            {
                outs << '-';
                val = (mask << 1) - val;
            }
        }
        outs << val;
    }
};



/**
 * Reads and formats an integral type of nbits bits.
 * The S template parameter selects between a signed
 * or unsigned type.
 */
template<int N, bool S> STRUCT IntHelper
{
    typedef typename TypeSelect<
        S,
        typename detail::int_type<N>::type,
        typename detail::int_type<N>::utype>::type type;

    static const bool is_signed = S;
    static const unsigned digits = std::numeric_limits<type>::digits;

    BOOST_STATIC_ASSERT(std::numeric_limits<type>::is_signed == S);

    static void inline
    print(std::ostream& outs, type val, int nbits, size_t offs, bool isChar)
    {
        IntOutputHelper<type, N>::print(outs, val, nbits, offs, isChar);
    }

    static void read
    (
        std::ostream&       outs,
        const DebugSymbol&  sym,
        int                 nbits,
        size_t              offs,
        bool                isChar
    )
    {
        assert(nbits);

        assert(nbits <= N);
        assert(!sym.is_return_value());

        const type x = Value<type>().read(sym);
        print(outs, x, nbits, offs, isChar);
    }


    static type from_string
    (
        const char* str,
        const char** p,
        int nbits,
        Unknown2* unk = 0
    )
    {
        assert(nbits);

        if (nbits <= (N / 2))
        {
            return IntHelper<N / 2, S>::from_string(str, p, nbits, unk);
        }
        assert(nbits <= N);
        return detail::input_traits<type>::from_string(str, p, unk);
    }


    static int compare(const char* lhs, const char* rhs, int nbits)
    {
        assert(nbits);

        if (nbits <= (N / 2))
        {
            return IntHelper<N / 2, S>::compare(lhs, rhs, nbits);
        }
        else
        {
            assert(nbits <= N);

            type mask = 1;
            mask <<= (nbits - 1);
            mask = ((mask - 1) << 1) | 1;

            type l = detail::input_traits<type>::from_string(lhs);
            type r = detail::input_traits<type>::from_string(rhs);

            l &= mask;
            r &= mask;

            if (l < r)
            {
                return -1;
            }
            if (l > r)
            {
                return 1;
            }
            return 0;
        }
    }
};




template<bool S>
STRUCT IntHelper<0, S>
{
    static int compare(const char*, const char*, int)
    { assert(false); return 0; }

    static int from_string(const char*, const char**, int, Unknown2*)
    { assert(false); return 0; }
};

#endif // INT_TYPES_H__C195A481_D518_4632_A3DE_FD3492756542
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
