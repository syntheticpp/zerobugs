#ifndef BYTESWAP_H__E8F7B3AF_45AE_413C_8E50_8399DACE87F6
#define BYTESWAP_H__E8F7B3AF_45AE_413C_8E50_8399DACE87F6
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include "zdk/export.h"

template<int N>
struct ZDK_LOCAL ByteSwap
{
    template<typename T>
    static void inline apply(T& x) { }
};

template<>
struct ZDK_LOCAL ByteSwap<16>
{
    template<typename T>
    static void inline apply(T& x)
    {
        x = ((x >> 8) & 0xff) |((x & 0xff) << 8);
    }
};

template<>
struct ZDK_LOCAL ByteSwap<32>
{
    template<typename T>
    static void inline apply(T& x)
    {
        x = ((x & 0xff000000) >> 24) |
            ((x & 0x00ff0000) >> 8 ) |
            ((x & 0x0000ff00) << 8 ) |
            ((x & 0x000000ff) << 24);
    }
};

template<>
struct ZDK_LOCAL ByteSwap<64>
{
    template<typename T>
    static void inline apply(T& x)
    {
        x =(((x & 0xff00000000000000ull) >> 56)
          | ((x & 0x00ff000000000000ull) >> 40)
          | ((x & 0x0000ff0000000000ull) >> 24)
          | ((x & 0x000000ff00000000ull) >> 8)
          | ((x & 0x00000000ff000000ull) << 8)
          | ((x & 0x0000000000ff0000ull) << 24)
          | ((x & 0x000000000000ff00ull) << 40)
          | ((x & 0x00000000000000ffull) << 56));
    }
};
#endif // BYTESWAP_H__E8F7B3AF_45AE_413C_8E50_8399DACE87F6
