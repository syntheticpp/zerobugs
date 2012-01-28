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
#include <sstream>
#include <stdexcept>
#include "zdk/shared_string_impl.h"
#include "zdk/type_system.h"
#include "zdk/unknown2.h"
#include "public/compile_unit.h"
#include "public/fwdtype.h"

#include "private/parse_events.h"
#include "private/throw.h"

#define this_func string(__func__)


using namespace std;


string Stab::typeid_to_str(const Stab::TypeID& typeID)
{
    ostringstream str;

    str << '(' << typeID.first << ',' << typeID.second << ')';
    return str.str();
}


void Stab::CompileUnit::parse(TypeSystem& types, ELF::Binary* elf)
{
    assert(beginIndex_);
    assert(endIndex_);
    assert(desc_);

    if (!beginIndex_)
    {
        THROW(logic_error(this_func + ": begin index not set"));
    }

    if (!endIndex_)
    {
        THROW(logic_error(this_func + ": end index not set"));
    }

    if (!desc_)
    {
        /*  todo: have ~Descriptor null out desc_ in all its
            CompileUnit children, to detect cases where
            pointers to CompileUnit objects are kept after
            the parent descriptor went out of scope. */

        THROW(logic_error(this_func + ": NULL descriptor"));
    }
    else if (!parsed_)
    {
        /*  set it before the actual parsing completes, to prevent
            infinite recursion. */

        parsed_ = true;

        if (!typeTables_.empty() && !typeTables_.front()->empty())
        {
            return;
        }

        // clog << __func__ << ": " << name().c_str() << endl;

        ParseEvents parse(types, *desc_);
        Events* events = &parse;

        if (elf)
        {
            desc_->for_each_stab(*elf, section(),
                                beginIndex_, endIndex_, &events, 1,
                                shortName_.get());
        }
        else
        {
            desc_->for_each_stab(section(),
                                beginIndex_, endIndex_, &events, 1,
                                shortName_.get());
        }
    }
}


void
Stab::CompileUnit::add_type(const TypeID& id, DataType* type)
{
    assert(type);
    assert(typeTables_.size() > id.first);

    if (!type || (typeTables_.size() <= id.first))
    {
        return;
    }
    TypeTablePtr table = typeTables_[id.first];

    if (table->size() <= id.second)
    {
        /* make room in table for the new type entry */
        table->resize(id.second + 1);
    }

    TypeTable::value_type& tptr = (*table)[id.second];

    /*  if there is an indirect type in that slot already,
        make it point to `type', so that all existing references
        to that fwd type will now link to `type'. */
    if (RefPtr<ForwardType> fwd = interface_cast<ForwardType>(tptr.ref_ptr()))
    {
        fwd->link(type);
    }

    assert(desc_);

    RefPtr<SharedString> typeName = type->name();

    Descriptor::FwdTypeMap& fwdTypes = desc_->forward_types();
    Descriptor::TypeMap& typeMap = desc_->type_map();

    if (ForwardType* fwd = interface_cast<ForwardType*>(type))
    {
        Descriptor::FwdTypeMap::iterator i = fwdTypes.find(typeName);
        if (i != fwdTypes.end())
        {
            type = fwd = i->second.get();
        }

        if (!fwd->link())
        {
            Descriptor::TypeMap::iterator j = typeMap.find(typeName);
            if (j != typeMap.end())
            {
                type = j->second.ref_ptr().get();
                fwd->link(type);
            }
        }

        if (i == fwdTypes.end())
        {
            /* Save forward type to map, for later. */
        #if HAVE_HASH_MAP
            fwdTypes.insert(make_pair(typeName, fwd));
        #else
            fwdTypes.insert(i, make_pair(typeName, fwd));
        #endif
        }
    }
    else
    {
        /* is there any forward reference to this type? */
        Descriptor::FwdTypeMap::iterator i = fwdTypes.find(typeName);
        if (i != fwdTypes.end())
        {
            i->second->link(type);
        }

        /* index the type by name */
        if (typeName->length() < 256)
        {
            typeMap.insert(make_pair(typeName, type));
        }
    }
    tptr = type;
}


RefPtr<DataType> Stab::CompileUnit::get_type
(
    TypeSystem& typeSys,
    const TypeID& id,
    bool follow
)
{
    if (typeTables_.size() <= id.first)
    {
        typeTables_.resize(id.first + 1);
    }
    TypeTablePtr& table = typeTables_[id.first];

    if (!table)
    {
        //table.reset(new TypeTable);
        // work around gcc 2.95 optimizer bug:
        TypeTablePtr(new TypeTable).swap(table);
    }
    if (id.second >= table->size())
    {
        table->resize(id.second + 1);
    }

    TypeTable::value_type& type = (*table)[id.second];
    if (!get_pointer(type))
    {
        const string typeID = typeid_to_str(id);

        type = typeSys.manage(new ForwardType(name().prepend(typeID.c_str()), id));
    }
    else if (follow)
    {
        if (RefPtr<ForwardType> fwd = interface_cast<ForwardType>(type.ref_ptr()))
        {
            if (fwd->link())
            {
                // NOTE: because `type' is a reference to a smart ptr,
                // this also replaces the table entry with the type
                // that `link' points to.
                type = fwd->link();
            }
        }
        // assert(get_pointer(type));
    }
    return CHKPTR(type.ref_ptr());
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
