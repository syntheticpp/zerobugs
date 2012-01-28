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

#if defined(USE_STLPORT) && (__GNUC__ > 2 || DEBUG)
// gcc 2.95 optimizer doesn't seem to grok hash containers
// (or maybe it runs out of memory?)
 #include <hash_map>
 #include <hash_set>
 #define HAVE_HASH_MAP 1
 #define ext std
#endif

#ifdef __INTEL_COMPILER
 #include <ext/hash_map>
#endif

#ifdef HAVE_HASH_MAP

#include "zdk/shared_string.h"
#include "zdk/uuid.h"

#if !defined(ext) && (defined (__GNUC__) && (__GNUC__ >= 3))
 #include <ext/hash_map>
 #include <ext/hash_set>

 namespace ext = __gnu_cxx;
#endif

#if (__GNUC__ < 4) || !defined(HAVE_GOOGLE_SPARSE_HASH)

namespace google
{
    /**
     * Using google::dense_hash_map when building with GCC 3.2.2 on RedHat9
     * causes a crash during the execution of global ctors. Pending
     * further investigation, fake the map with "standard" hash_map
     */
    template<typename Key, typename Val>
    struct dense_hash_map : public ext::hash_map<Key, Val>
    {
        typedef ext::hash_map<Key, Val> base_map;
        typedef typename base_map::iterator iterator;

        void set_empty_key(Key) { }

        std::pair<iterator, bool> insert(iterator, std::pair<Key, Val> val)
        {
            return base_map::insert(val);
        }
        std::pair<iterator, bool> insert(std::pair<Key, Val> val)
        {
            return base_map::insert(val);
        }
    };
}
#else
  #include <google/dense_hash_map>
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

#else // do not have hash_map, use map instead

#define ext std
#include <map>
 #define hash_map map
 #define hash_multimap multimap
 #define hash_set set
 #define hash_multiset multiset
#endif

#endif // HASH_MAP_H__878BD04D_AAD2_4A03_8FF2_CB029D43209C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
