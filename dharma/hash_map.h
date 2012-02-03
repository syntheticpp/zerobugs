#ifndef HASH_MAP_H__878BD04D_AAD2_4A03_8FF2_CB029D43209C
#define HASH_MAP_H__878BD04D_AAD2_4A03_8FF2_CB029D43209C
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// --------------------------------------------------------------------------

#include "zdk/hsieh.h"

#define HAVE_HASH_MAP 1

#ifdef __INTEL_COMPILER
 #include <ext/hash_map>
#endif

#include "zdk/shared_string.h"
#include "zdk/uuid.h"

#if defined (__GNUC__)
 #include <ext/hash_map>
 #include <ext/hash_set>

 namespace ext = __gnu_cxx;
#endif


namespace __gnu_cxx
{
    template<> struct hash<std::string>
    {
        int operator()(const std::string& str) const
        {
            return SuperFastHash(str.c_str(), str.length());
        }
    };

    template<> struct hash<RefPtr<SharedString> >
    {
        int operator()(const RefPtr<SharedString>& str) const
        {
            assert(str);

            return SuperFastHash(str->c_str(), str->length());
        }
    };

    template<> struct hash<ZDK_UUID>
    {
        int operator()(const ZDK_UUID& uuid) const
        {
            return SuperFastHash(reinterpret_cast<const char*>(&uuid), sizeof (ZDK_UUID));
        }
    };
} // namespace


#endif // HASH_MAP_H__878BD04D_AAD2_4A03_8FF2_CB029D43209C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
