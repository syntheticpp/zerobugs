//
// $Id: factory.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "public/error.h"
#include "public/producer.h"
#include "public/utils.h"
#include "public/impl.h"
#include "private/factory.h"
#include "private/log.h"
#include "generic/lock.h"


using namespace std;
using namespace Dwarf;


Factory::Factory()
{
}


Factory::~Factory()
{
}


Factory& Factory::instance()
{
    static Factory die_factory;
    return die_factory;
}


bool Factory::register_creator(Dwarf_Half tag, Creator* creator)
{
    Lock<Mutex> lock(mutex_);

    bool res = creatorMap_.insert(make_pair(tag, creator)).second;
    return res;
}


boost::shared_ptr<Die> 
Factory::create(
    Dwarf_Debug dbg,
    Dwarf_Die   die,
    bool        own,
    Dwarf_Half* tagRet
    ) const
{
    if (Dwarf_Half tag = Utils::tag(dbg, die, nothrow))
    {
        if (tagRet)
        {
            *tagRet = tag;
        }
        return create(dbg, die, tag, own);
    }
    return boost::shared_ptr<Die>();
}


boost::shared_ptr<Die>
Factory::create(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half tag, bool own) const
{
    if (!tag)
    {
        tag = Utils::tag(dbg, die);
    }
    boost::shared_ptr<Die> instance;

    Lock<Mutex> lock(mutex_);

    CreatorMap::const_iterator i = creatorMap_.find(tag);
    if (i != creatorMap_.end())
    {
        instance = (*i->second)(dbg, die);
    }
    else
    {
        log<warn>() << "no creator registered for DWARF_TAG=0x"
                    << hex << tag << dec << "\n";

        if (own)
        {
            dwarf_dealloc(dbg, die, DW_DLA_DIE);
        }
        // avoid the warning next time
        Producer<Die, 0>::register_with_factory(tag);
    }
    return instance;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
