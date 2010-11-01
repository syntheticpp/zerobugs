//
// $Id: object_manager.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include "object_manager.h"
#include "unmangle/unmangle.h"

using namespace std;

ObjectManager* ObjectManagerFactory::instance_ = NULL;

ObjectManagerImpl::ObjectManagerImpl()
{
    cout << "ObjectManagerImpl\n";
}

ObjectManagerImpl::~ObjectManagerImpl()
{
    cout << "~ObjectManagerImpl\n";
}


////////////////////////////////////////////////////////////////
ostream& ObjectManagerImpl::print(ostream& outs, const char* func) const
{
    if (!instanceMap_.empty())
    {
        outs << "#############################################\n";
        outs << func << endl;
        outs << "InstanceCounted objects in use:\n";

        instance_map_type::const_iterator i(instanceMap_.begin());

        for (; i != instanceMap_.end(); ++i)
        {
        #if (__GNUC__ < 3)
            // hackish demangling:
            size_t n = i->first.find_last_of("0123456789");

            outs << i->first.substr(++n) << ": " << i->second.first;
        #else
            //outs << cplus_unmangle(("_Z" + i->first), UNMANGLE_NOFUNARGS);
            outs << i->first;
            outs << ": " << i->second.first;
        #endif
            outs << " object(s), " << i->second.second << " byte(s)\n";
        }
        outs << "#############################################\n";
    }

    return outs;
}


void ObjectManagerImpl::remove(instance_map_type::iterator i, size_t size)
{
    assert (i != instanceMap_.end());
    if (i != instanceMap_.end())
    {
        assert(i->second.first > 0);
        if (--i->second.first == 0)
        {
            instanceMap_.erase(i);
        }
        else
        {
            assert(i->second.second >= size);
            i->second.second -= size;
        }
    }
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
