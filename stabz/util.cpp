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
#include <cstring>
#include <cstdlib>
#include <map>
#include <string>
#include <stab.h>
#include "private/util.h"

using namespace std;


const char* Stab::get_name(int type)
{
    static map<int, const char*> names;

    if (names.empty())
    {
#ifdef __linux__
 #define __define_stab(code, val, str) names[val] = str;
 #include <bits/stab.def>
#else
        names[N_GSYM] = "GSYM";
        names[N_FNAME] = "FNAME";
        names[N_FUN] = "FUN";
        names[N_STSYM] = "STSYM";
        names[N_LCSYM] = "LCSYM";
        names[N_MAIN] = "MAIN";
        names[N_PC] = "PC";	/* global Pascal symbol */
        names[N_RSYM] = "RSYM";
        names[N_SLINE] = "SLINE";
        names[N_DSLINE] = "DSLINE";
        names[N_BSLINE] = "BSLINE";
        names[N_SSYM] = "SSYM"; /* structure/union element */
        names[N_SO] = "SO";
        names[N_LSYM] = "LSYM";
        names[N_BINCL] = "BINCL";
        names[N_SOL] = "SOL";
        names[N_PSYM] = "PSYM";
        names[N_EINCL] = "EINCL";
        names[N_ENTRY] = "RNTRY";
        names[N_LBRAC] = "LBRAC";
        names[N_EXCL] = "EXCL";
        names[N_RBRAC] = "RBRAC";
        names[N_BCOMM] = "BCOMM";
        names[N_ECOMM] = "ECOMM";
        names[N_ECOML] = "ECOML";
        names[N_LENG] = "LENG"; /* length of preceding entry */
#endif // !linux GNU LIBC
    }

    map<int, const char*>::const_iterator i = names.find(type);
    if (i != names.end())
    {
        return i->second;
    }

    return 0;
}


char* Stab::fullpath(const char* prefix, const char* path)
{
    assert(path);

    char* result = 0;

    if (*path == '/' || prefix == 0 || *prefix != '/')
    {
        result = strdup(path);
    }
    else
    {
        assert(prefix);
        assert(*prefix == '/');

        size_t len = strlen(prefix);

        /*  allocate an extra byte for the nul terminator,
            and another just in case a path delimiter needs
            to be added. */
        result = static_cast<char*>(malloc(len + strlen(path) + 2));

        memcpy(result, prefix, len);

        if (result[len - 1] != '/')
        {
            result[len++] = '/';
        }

        strcpy(result + len, path);
    }

    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
