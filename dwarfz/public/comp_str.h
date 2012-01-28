#ifndef COMP_STR_H__73B6954C_67F7_45DA_B840_D5129E8E2B5A
#define COMP_STR_H__73B6954C_67F7_45DA_B840_D5129E8E2B5A
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

#include <functional>
#include "zdk/string.h"


namespace Dwarf
{
    struct StrLess : public std::binary_function<
        const char*, const char*, bool>
    {
        bool operator()(const char* lhs, const char* rhs) const
        {
            //return strcmp(lhs, rhs) < 0;
            return strcmp_ignore_space(lhs, rhs) < 0;
        }
    };

    struct StrEqual : public std::binary_function<
        const char*, const char*, bool>
    {
        bool operator()(const char* lhs, const char* rhs) const
        {
            //return strcmp(lhs, rhs) == 0;
            return strcmp_ignore_space(lhs, rhs) == 0;
        }
    };

    inline size_t
    hash_ignore_space(const char* __s)
    {
        unsigned long __h = 0;
        for ( ; *__s; ++__s)
        {
            if (isspace(*__s))
                continue;
            __h = 5 * __h + *__s;
        }
        return size_t(__h);
    }

    struct StrHash
    {
        size_t operator()(const char* s) const
        {
            return hash_ignore_space(s);
        }
    };
}
#endif // COMP_STR_H__73B6954C_67F7_45DA_B840_D5129E8E2B5A
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
