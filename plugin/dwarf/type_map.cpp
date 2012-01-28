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
#include <string>
#include "zdk/interface_cast.h"
#include "zdk/shared_string_impl.h"
#include "zdk/ref_ptr.h"
#include "dwarfz/public/debug.h"
#include "dwarfz/public/die.h"
#include "dwarfz/public/type.h"
#include "typez/public/data_type_impl.h"
#include "type_map.h"


using namespace std;
using namespace Dwarf;


TypeMap::~TypeMap()
{
//#ifdef DEBUG
//    clog << "type_map: byName size=" << byName_.size() << endl;
//    clog << "type_map: byOffs size=" << byOffset_.size() << endl;
//#endif
}


RefPtr<DataType>
TypeMap::add_internal(const Dwarf::Die& die,
                      const RefPtr<DataType>& type,
                      bool indexByName)
{
    assert(type);
    assert(type->ref_count() > 1);

    if (type.is_null())
    {
        return RefPtr<DataType>(); // NULL
    }

    if (indexByName && die.name() && *die.name())
    {
        assert(*CHKPTR(type->name()) != *unnamed_type());
        byName_.insert(make_pair(type->name(), type.get()));
    }
    pair<ino_t, Dwarf_Off> key(die.owner().inode(), die.offset());

    ByOffset::iterator i =
        byOffset_.insert(make_pair(key, type.get())).first;

    if (i->second.ref_ptr() != type)
    {
        i->second = type;
    }
    return i->second.ref_ptr();
}


RefPtr<DataType> TypeMap::find(const RefPtr<SharedString>& name) const
{
    RefPtr<DataType> type;

    ByName::const_iterator i = byName_.find(name);
    if (i != byName_.end())
    {
        type = i->second.ref_ptr();
    }
    return type;
}


RefPtr<DataType> TypeMap::add(
    const Dwarf::Type& type,
    const RefPtr<DataType>& dataType,
    bool indexByName)
{
    if (type.is_incomplete())
    {
        indexByName = false;
    }
    return add_internal(type, dataType, indexByName);
}


RefPtr<DataType> TypeMap::find(const char* name) const
{
    RefPtr<SharedString> key = shared_string(name);
    return find(key);
}


RefPtr<DataType> TypeMap::find(const Dwarf::Die& die) const
{
    RefPtr<DataType> type;

    pair<ino_t, Dwarf_Off> key(die.owner().inode(), die.offset());

    ByOffset::const_iterator i = byOffset_.find(key);
    if (i != byOffset_.end())
    {
        type = i->second.ref_ptr();
    }
    return type;
}


void TypeMap::clear()
{
    byName_.clear();
    byOffset_.clear();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
