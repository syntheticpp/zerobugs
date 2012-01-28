//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include <iostream>
#include <set>
#include "generic/lock.h"
#include "public/string_pool.h"
#include "zdk/shared_string_impl.h"
#include "dharma/environ.h"

using namespace std;


static const size_t THRESHOLD = 8;
static const bool printStats = env::get_bool("ZERO_STRING_CACHE_STATS", false);


StringPool::StringPool() : savings_(0)
{
}


StringPool::~StringPool() throw()
{
    if (printStats)
    {
        print_stats(clog);
    }
}


SharedString*
StringPool::get_string(const char* s, size_t len)
{
    if (!s)
    {
        s = "";
        assert(len == 0);
    }
    else if (!len)
    {
        len = strlen(s);
    }
#if 0 
    // TODO: re-visit this code, I suspect a bug in here
    if (len > THRESHOLD)
    {
        Lock<Mutex> lock(mutex_);

        const size_t hash = SharedStringImpl::hash(s);
        return buckets_[hash].get_string(s, len, hash, savings_);
    }
#endif
    return SharedStringImpl::create(s, len);
}


SharedString* StringPool::get_string(SharedString* str)
{
    size_t hash = 0;
    if (str)
    {
        if (str->length() <= THRESHOLD)
        {
            return str;
        }
        hash = str->hash();
    }
    Lock<Mutex> lock(mutex_);
    return buckets_[hash].get_string(str, savings_);
}


StringPool::Bucket::Bucket()
{
}



SharedString*
StringPool::Bucket::get_string(const char* s,
                               size_t len,
                               size_t hash,
                               size_t& savings)
{
    for (StringList::iterator i = strings_.begin(); i != strings_.end();)
    {
        if (RefPtr<SharedString> ss = i->ref_ptr())
        {
            ++i;

            if (ss->length() != len)
                continue;

            if (ss->hash() != hash)
                continue;

            if (ss->is_equal(s))
            {
                savings += len;
                return ss.get();
            }
        }
        else
        {
            i = strings_.erase(i);
        }
    }
    SharedString* str = SharedStringImpl::create(s, len);
    strings_.push_back(str);
    return str;
}



SharedString*
StringPool::Bucket::get_string(SharedString* s, size_t& savings)
{
    for (StringList::iterator i = strings_.begin(); i != strings_.end();)
    {
        if (RefPtr<SharedString> ss = i->ref_ptr())
        {
            ++i;

            if (ss->is_equal2(s))
            {
                savings += ss->length();
                return ss.get();
            }
        }
        else
        {
            i = strings_.erase(i);
        }
    }
    strings_.push_back(s);
    return s;
}


void StringPool::print_stats(ostream& os)
{
    os << "==============================================\n";
    os << " String Pool: " << buckets_.size() << " buckets\n";
    os << " Total savings (in bytes): " << savings_ << "\n";
    os << "==============================================\n";
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
