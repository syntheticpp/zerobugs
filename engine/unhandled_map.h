#ifndef UNHANDLED_MAP_H__05A306DF_1F18_4C3A_8C5A_99C47F9C6B1C
#define UNHANDLED_MAP_H__05A306DF_1F18_4C3A_8C5A_99C47F9C6B1C
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

#include <map>
#include "zdk/zobject_impl.h"

/**
 * Maintain a mapping from unhandled PIDs to status int-s
 */
class UnhandledMap : public ZObjectImpl<>
{
public:
    typedef std::map<pid_t, int> map_type;
    typedef map_type::iterator iterator;

DECLARE_UUID("e19fd7cc-b716-4b14-b833-755233ce2b26")
BEGIN_INTERFACE_MAP(UnhandledMap)
    INTERFACE_ENTRY(UnhandledMap)
END_INTERFACE_MAP()

    virtual ~UnhandledMap() throw ();

    pid_t query_status(pid_t, int* = NULL, bool remove = false);

    void remove_status(pid_t);

    void add_status(pid_t, int);

    iterator begin() { return map_.begin(); }
    iterator end() { return map_.end(); }

    bool empty() const { return map_.empty(); }
    size_t size() const { return map_.size(); }

    void erase(iterator i) { map_.erase(i); }

private:
    map_type map_;
};


#endif // UNHANDLED_MAP_H__05A306DF_1F18_4C3A_8C5A_99C47F9C6B1C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
