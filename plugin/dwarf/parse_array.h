#ifndef PARSE_ARRAY_H__FC1B2E1E_6E66_4E8F_AEAA_5C0906669DB1
#define PARSE_ARRAY_H__FC1B2E1E_6E66_4E8F_AEAA_5C0906669DB1
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

#include "zdk/type_tags.h"


inline size_t
ZDK_LOCAL
parse_array(const char* str, Unknown2* unk, int wordSize)
{
    char* ptr = 0;
    addr_t addr = 0;

    if (strcmp(str, "null") == 0)
    {
        ptr = const_cast<char*>(str) + 4;
    }
    else
    {
        addr = strtoul(str, &ptr, 0);
    }
    if (wordSize == 32)
    {
        put(unk, uint32_t(addr), Variant::VT_ARRAY);
    }
    else
    {
        put(unk, addr, Variant::VT_ARRAY);
    }
    if (ptr)
    {
        assert(ptr >= str);
        return ptr - str;
    }
    return 0;
}

#endif // PARSE_ARRAY_H__FC1B2E1E_6E66_4E8F_AEAA_5C0906669DB1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
