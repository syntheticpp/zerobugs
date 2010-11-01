#ifndef STRING_CACHE_H__69588154_EAEB_4AB6_9523_CA18625B890D
#define STRING_CACHE_H__69588154_EAEB_4AB6_9523_CA18625B890D
//
// $Id: string_cache.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/ref_ptr.h"
#include "zdk/shared_string.h"


DECLARE_ZDK_INTERFACE_(StringCache, ZObject)
{
    DECLARE_UUID("106b26a0-b1db-4626-8b93-a192d882dcf0")

    virtual SharedString* get_string(const char*, size_t len = 0) = 0;

    virtual SharedString* get_string(SharedString*) = 0;
};


#endif // STRING_CACHE_H__69588154_EAEB_4AB6_9523_CA18625B890D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
