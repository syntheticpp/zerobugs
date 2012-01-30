#ifndef ARGV_UTIL_H__DDC8C327_8663_44A8_8A46_3FCC9720FB83
#define ARGV_UTIL_H__DDC8C327_8663_44A8_8A46_3FCC9720FB83
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

#include "zdk/export.h"
#include "zdk/stdexcept.h"
#include <assert.h>
#include <string.h>
#include <string>
#include <boost/lexical_cast.hpp>

/**
 * Shift arguments to the left, starting at index i
 */
void inline ZDK_LOCAL argv_shift(int* argc, char** argv, int i)
{
    assert(argc);
    assert(argv);

    --*argc;
    while (i++ < *argc)
    {
        *argv = *(argv + 1);
        ++argv;
    }
}


bool inline ZDK_LOCAL argv_match(
    int*            argc,
    char***         argv,
    int             i,
    const char*     s)
{
    assert(argc);
    assert(argv);
    assert(*argv);
    assert(i < *argc);

    char** p = &(*argv)[i];

    if (strcmp(*p, s) == 0)
    {
        argv_shift(argc, p, i);
        return true;
    }
    return false;
}


bool inline ZDK_LOCAL argv_match(
    int*            argc,
    char***         argv,
    int             i,
    const char*     s1,
    const char*     s2)
{
    assert(argc);
    assert(argv);
    assert(*argv);
    assert(i < *argc);

    char** p = &(*argv)[i];

    if (strcmp(*p, s1) == 0 || strcmp(*p, s2) == 0)
    {
        argv_shift(argc, p, i);
        return true;
    }
    return false;
}


template<typename T>
struct ZDK_LOCAL argv_traits
{
    static inline void read(const char* arg, T& val)
    {
        val = boost::lexical_cast<T>(arg);
    }
};

template<> struct ZDK_LOCAL argv_traits<const char*>
{
    static inline void read(const char* arg, const char*& val)
    {
        val = arg;
    }
};

template<> struct ZDK_LOCAL argv_traits<std::string>
{
    static inline void read(const char* arg, std::string& val)
    {
        if (arg)
        {
            val.assign(arg);
        }
    }
};

template<typename T> bool inline ZDK_LOCAL argv_match(

    int*        argc,
    char***     argv,
    int         i,
    const char* s,
    size_t      n,
    T&          val)
{
    assert(argc);
    assert(argv);
    assert(*argv);
    assert(i < *argc);

    char** p = &(*argv)[i];

    if (strncmp(*p, s, n) == 0)
    {
        argv_traits<T>::read(*p + n, val);
        argv_shift(argc, p, i);
        return true;
    }
    return false;
}


/*------------------------------------------------------------*/
/* Convenience macros. Intended use:

 BEGIN_ARG_PARSE(argc, argv)
    ON_ARG("foo")
        { // ...
        }
    ON_ARG("bar")
        { // ...
        }
 END_ARG_PARSE

 */
#define BEGIN_ARG_PARSE(c,v) {              \
    assert(c);                              \
    assert(v);                              \
    int* _ac = (c); char*** _av = (v);    \
    for (int i = 1; i < *_ac; ) { if (!i){} \

#define ON_ARG(s) else if (argv_match(_ac, _av, i, (s)))

#define ON_ARGV(s,v) \
    else if (argv_match(_ac, _av, i, (s), sizeof(s) - 1, v))

#define END_ARG_PARSE else ++i; } }

/*------------------------------------------------------------*/


#endif // ARGV_UTIL_H__DDC8C327_8663_44A8_8A46_3FCC9720FB83
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
