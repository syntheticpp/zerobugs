#ifndef STRING_POOL_H__928F911B_7EB2_47DB_8131_67FE726360BF
#define STRING_POOL_H__928F911B_7EB2_47DB_8131_67FE726360BF
//
// $Id: string_pool.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/mutex.h"
#include "zdk/string_cache.h"
#include "zdk/weak_ptr.h"
#include "zdk/zobject_impl.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include <vector>


/**
 * Attempt to minimize memory spent in strings, by
 * reducing duplicates
 */
class ZDK_LOCAL StringPool : public ZObjectImpl<StringCache>
{
public:
    StringPool();
    ~StringPool() throw();

    void print_stats(std::ostream&);


    class Bucket
    {
        typedef std::vector<WeakPtr<SharedString> > StringList;
        StringList strings_;

    public:
        Bucket();

        SharedString* get_string(const char* str,
                                 size_t len,
                                 size_t hash,
                                 size_t& totalSavings);

        SharedString* get_string(SharedString*, size_t& totalSavings);
    }; // Bucket

    typedef ext::hash_map<size_t, Bucket> BucketMap;

private:
    virtual SharedString* get_string(const char*, size_t len = 0);
    virtual SharedString* get_string(SharedString*);

    BucketMap buckets_;
    size_t savings_;
    Mutex mutex_;
};

#endif // STRING_POOL_H__928F911B_7EB2_47DB_8131_67FE726360BF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
