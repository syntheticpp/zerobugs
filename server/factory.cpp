// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: factory.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "factory.h"
using namespace std;

void Factory::register_interface(uuidref_t iid, CreatorFunc fn)
{
    assert(iid);
    factoryMap_.insert(make_pair(*iid, fn));
}


bool Factory::unregister_interface(uuidref_t iid)
{
    assert(iid);
    FactoryMap::iterator i = factoryMap_.find(*iid);
    if (i != factoryMap_.end())
    {
        factoryMap_.erase(i);
        return true;
    }
    return false;
}


ZObject* Factory::create_object(uuidref_t iid, const char* name)
{
    assert(iid);
    FactoryMap::const_iterator i = factoryMap_.find(*iid);
    if (i != factoryMap_.end())
    {
        return (*i->second)(this, name);
    }
    return NULL;
}
