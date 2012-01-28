#ifndef FHEAP_H__3F1E994F_B86B_43F4_B99C_6B78C8F6B23B
#define FHEAP_H__3F1E994F_B86B_43F4_B99C_6B78C8F6B23B
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

#include "zdk/falloc.h"

template<size_t K = 4>
struct ZDK_LOCAL Fheap
{
    static Falloc<K * 1024> falloc;

    static void* allocate(size_t n)
    { return falloc.allocate(n); }

    static void dealocate(void*p, size_t n)
    { falloc.dealocate(p, n); }

    static size_t free_bytes()
    { return falloc.free_bytes(); }

    static size_t used_bytes()
    { return falloc.used_bytes(); }

    static void drop() { falloc.drop(); }

    static size_t max_size()
    { return falloc.max_size(); }
};


#endif // FHEAP_H__3F1E994F_B86B_43F4_B99C_6B78C8F6B23B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
