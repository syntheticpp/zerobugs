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

#include <stdexcept>
#include "unhandled_map.h"


UnhandledMap::~UnhandledMap() throw ()
{
}


void UnhandledMap::add_status(pid_t pid, int status)
{
    assert(pid);

    map_[pid] = status;
}


pid_t UnhandledMap::query_status(pid_t pid, int* status, bool remove)
{
    assert(pid);

    map_type::iterator i = map_.begin();

    if (pid != -1)
    {
        i = map_.find(pid);
    }
    if (i == map_.end())
    {
        return 0;
    }
    if (status)
    {
        *status = i->second;
    }

    pid = i->first;

    if (remove)
    {
        map_.erase(i);
        assert(map_.find(pid) == map_.end());
    }
    return pid;
}


void UnhandledMap::remove_status(pid_t pid)
{
    map_type::iterator i = map_.find(pid);
    if (i == map_.end())
    {
        throw std::invalid_argument(__func__);
    }
    map_.erase(i);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
