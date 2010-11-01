#ifndef CASE_INSENSITIVE_H__079B24C0_5145_4E99_8721_9EA338573AB4
#define CASE_INSENSITIVE_H__079B24C0_5145_4E99_8721_9EA338573AB4
//
// $Id: case_insensitive.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string> // for std::char_traits

struct CaseInsensitiveTraits : public std::char_traits<char>
{
    static bool eq(const char& c1, const char& c2)
    { return tolower(c1) == tolower(c2); }

    static int compare(const char* s1, const char* s2, size_t n)
    { return strncasecmp(s1, s2, n); }
};

typedef std::basic_string<char, CaseInsensitiveTraits> CaseInsensitiveString;

#endif // CASE_INSENSITIVE_H__079B24C0_5145_4E99_8721_9EA338573AB4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
