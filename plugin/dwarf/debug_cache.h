#ifndef HANDLE_CACHE_H__4C1F541B_356D_4C6D_82C3_1C09EA1293A4
#define HANDLE_CACHE_H__4C1F541B_356D_4C6D_82C3_1C09EA1293A4
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

#include <set>
#include <string>
#include <vector>
#include "zdk/mutex.h"
#include "zdk/shared_string.h"
#include "zdk/zerofwd.h"
#include "dwarfz/public/debug.h"


/**
 * Caches Dwarf::Debug handles by filename
 */
CLASS DebugCache
{
    struct Entry
    {
        Entry() : usageCount_(0) {}

        size_t usageCount_;
        RefPtr<SharedString> fileName_;
        std::shared_ptr<Dwarf::Debug> handle_;
    };
    typedef std::set<RefPtr<SharedString> > BlackList;

    std::vector<Entry> cache_;
    mutable Mutex mutex_;

    BlackList blackList_;
    Debugger* debugger_;

    static bool show_status(void*, const char*);

public:
    typedef std::shared_ptr<Dwarf::Debug> Handle;

    /**
     * Because the operating system limits the number of
     * open file descriptors, we have to impose a maxSize
     * number of entries -- when exceeded, the entry with
     * the least usageCount is discarded and replaced.
     */
    DebugCache(size_t maxSize, Debugger*);

    virtual ~DebugCache();

    Handle get_handle(const RefPtr<SharedString>&,
                      const RefPtr<Process>&,
                      const Dwarf::UnitHeadersCallback*);

    Handle get_handle(ino_t, Process*) const;

};

#endif // HANDLE_CACHE_H__4C1F541B_356D_4C6D_82C3_1C09EA1293A4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
