#ifndef PATH_H__F7EEAD74_B0D2_48D6_85F2_6EA8F430F0DF
#define PATH_H__F7EEAD74_B0D2_48D6_85F2_6EA8F430F0DF
//
// $Id: path.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#ifdef basename
 #undef basename
#endif
#include <string.h>
#include <string>

#if defined (HAVE_LIBGEN_H) && !defined (__linux__)
 #include <libgen.h>

// not including / using libgen.h on Linux, because
// both __xpg_basename and dirname are const-broken,
// but <string.h> provides a correct prototype

#endif

namespace sys
{
    inline std::string dirname(const char* path)
    {
        if (const char* p = strrchr(path, '/'))
        {
            return std::string(path, p - path);
        }
        return std::string(".");
    }
}

#endif // PATH_H__F7EEAD74_B0D2_48D6_85F2_6EA8F430F0DF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
