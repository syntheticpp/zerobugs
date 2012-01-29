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
#include <iostream>
#include <boost/limits.hpp>
#include "debug_cache.h"
#include "dbgout.h"
#include "elfz/public/binary.h"
#include "generic/lock.h"
#include "zdk/type_system.h"
#include "zdk/zero.h"
#include "dharma/environ.h"
#include "dwarfz/public/compile_unit.h"

using namespace std;
using namespace Dwarf;


static void auto_enable_d_lang_support(Dwarf::Debug& debug)
{
    if (!env::get_bool("ZERO_D_SUPPORT"))
    {
        Debug::UnitList::const_iterator u = debug.units().begin();

        for (; u != debug.units().end(); ++u)
        {
            if ((*u)->language() == DW_LANG_D
                || strncmp((*u)->producer(), "Digital Mars D", 14) == 0
                || strstr((*u)->producer(), "using dmd"))
            {
                cout << "*** Digital Mars D detected ***\n";
                //
                // Set the environment variable without overriding it;
                // setting it to false in the shell takes precedence.
                //
                env::set("ZERO_D_SUPPORT", true, false);
                break;
            }
        }
    }
}


DebugCache::DebugCache
(
    size_t maxSize,
    Debugger* dbg
) : cache_(maxSize), debugger_(dbg)
{
    dbgout(1) << "DebugCache size=" << maxSize << endl;
}


DebugCache::~DebugCache()
{
}


static void
check_gnu_debuglink(DebugCache::Handle& handle,
                    const RefPtr<SharedString>& path)
{
    if (handle->is_null() || handle->empty())
    {
        // look for a gnu-debuglink section
        ELF::Binary binary(path->c_str());

        const string link = ELF::debug_link(binary);
        if (!link.empty())
        {
            dbgout(0) << __func__ << ": " << path->c_str() << " -> " << link << endl;
            handle.reset(new Dwarf::Debug(link.c_str()));
        }
    }
}


DebugCache::Handle
DebugCache::get_handle(const RefPtr<SharedString>& path,
                       const RefPtr<Process>& process,
                       const UnitHeadersCallback* callback)
{
    if (!path)
    {
        dbgout(1) << __func__ << ": NULL path" << endl;
        return Handle();
    }
    dbgout(2) << __func__ << ": " << path.get() << endl;
    Lock<Mutex> lock(mutex_);

    if (blackList_.find(path) != blackList_.end())
    {
        dbgout(0) << ": black-listed: " << path << endl;
        return Handle();
    }

    for (size_t i = 0; i != cache_.size(); ++i)
    {
        if (cache_[i].fileName_
         && path->is_equal2(cache_[i].fileName_.get()))
        {
            if (cache_[i].handle_.get())
            {
                ++cache_[i].usageCount_;
            }
            return cache_[i].handle_; // found
        }
    }
    // not found, make a new handle
    Handle handle(new Dwarf::Debug(path->c_str(), callback));
    check_gnu_debuglink(handle, path);

    if (handle)
    {
        if (handle->is_null())
        {
            handle.reset();
        }
        else if (TypeSystem* typeSystem = interface_cast<TypeSystem*>(process.get()))
        {
            handle->set_string_cache(typeSystem);
        }
    }
    // find the least used slot
    size_t slot = 0;
    size_t minUsage = std::numeric_limits<size_t>::max();

    for (size_t i = 0; i != cache_.size(); ++i)
    {
        if (cache_[i].usageCount_ < minUsage)
        {
            minUsage = cache_[i].usageCount_;
            slot = i;
        }
    }
    Entry& entry = cache_[slot];
    entry.handle_ = handle;
    entry.fileName_ = path;
    entry.usageCount_ = 1;

    if (handle)
    {
        auto_enable_d_lang_support(*handle);
    }
    else
    {
        blackList_.insert(path);
    }
    return handle;
}


/**
 * Lookup handle by inode
 */
DebugCache::Handle
DebugCache::get_handle(ino_t inode, Process* /* process */) const
{
    Lock<Mutex> lock(mutex_);
    Handle handle;

    for (size_t i = 0; i != cache_.size(); ++i)
    {
        const Entry& entry = cache_[i];
        if (entry.handle_ && entry.handle_->inode() == inode)
        {
            handle = entry.handle_;
            break;
        }
    }
    return handle;
}



bool DebugCache::show_status(void* ptr, const char* msg)
{
    if (ptr)
    {
        reinterpret_cast<Debugger*>(ptr)->message(msg, Debugger::MSG_STATUS);
    }
    return true;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
