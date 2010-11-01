//
// $Id: class_type.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <stdexcept>
#include "factory.h"
#include "generic_attr.h"
#include "inheritance.h"
#include "location.h"
#include "member.h"
#include "utils.h"
#include "impl.h"
#include "class_type.h"


using namespace Dwarf;


StructTypeBase::StructTypeBase(Dwarf_Debug dbg, Dwarf_Die die)
    : Type(dbg, die)
{
}


List<DataMember> StructTypeBase::members() const
{
    return List<DataMember>(dbg(), die());
}


const StructTypeBase::StaticMemData&
StructTypeBase::static_members() const
{
    if (!staticMembers_.get())
    {
        staticMembers_.reset(new StaticMemData);

        List<StaticMember> statics(dbg(), die());
        List<StaticMember>::const_iterator i = statics.begin(),
                                         end = statics.end();
        for (; i != end; ++i)
        {
            staticMembers_->push_back(i);
        }
    }
    return *staticMembers_;
}


KlassType::KlassType(Dwarf_Debug dbg, Dwarf_Die die)
    : StructTypeBase(dbg, die)
{
    const Dwarf_Half tag = Utils::tag(dbg, die);
    assert(tag == DW_TAG_structure_type || tag == DW_TAG_class_type);

    if (tag != DW_TAG_structure_type && tag != DW_TAG_class_type)
    {
        throw std::logic_error("class type with invalid tag");
    }
}


const KlassType::BaseList& KlassType::bases() const
{
    if (!bases_.get())
    {
        bases_.reset(new BaseList);
        List<Inheritance> bases(dbg(), die());

        List<Inheritance>::const_iterator i(bases.begin());
        const List<Inheritance>::const_iterator end(bases.end());
        for (; i != end; ++i)
        {
            bases_->push_back(i);
        }
    }
    return *bases_;
}


const MethodList& KlassType::methods() const
{
    if (!methods_.get())
    {
        methods_.reset(new MethodList);
        List<MemFun> memFuncs(dbg(), die());

        List<MemFun>::const_iterator i(memFuncs.begin());
        const List<MemFun>::const_iterator end(memFuncs.end());
        for (; i != end; ++i)
        {
            const MemFun& fun = *i;

            if (fun.param_types().empty())
            {
                // methods should have at least one param -- `this'
                continue;
            }
        #if 0
            // non-member operators may appear here, they don't have
            // a `this' artificial parameter -- don't add them as methods
            if (!fun.param_types()[0]->is_artificial())
            {
                continue;
            }
        #endif
            methods_->push_back(i);
        }
    }
    return *methods_;
}


UnionType::UnionType(Dwarf_Debug dbg, Dwarf_Die die)
    : StructTypeBase(dbg, die)
{
    const Dwarf_Half tag = Utils::tag(dbg, die);
    if (tag != DW_TAG_union_type)
    {
        throw std::logic_error("DW_TAG_union_type expected");
    }
}


bool MemFun::is_virtual() const
{
    if (!Utils::has_attr(dbg(), die(), DW_AT_virtuality))
    {
        return false;
    }
    GenericAttr<DW_AT_virtuality, Dwarf_Unsigned> attr(dbg(), die());
    return attr.value() != DW_VIRTUALITY_none;
}



Dwarf_Addr MemFun::vtable_offset(Dwarf_Addr frameBase,
                                 Dwarf_Addr moduleBase,
                                 Dwarf_Addr unitBase,
                                 Dwarf_Addr programCount) const
{
    Dwarf_Addr result = 0;

    if (boost::shared_ptr<Location> loc =
        Utils::loc(dbg(), die(), DW_AT_vtable_elem_location))
    {
    /*
        dlog(L_DBG) << __func__ << ": module_base="
                    << (void*)moduleBase << " unit_base="
                    << (void*)unitBase << " pc="
                    << (void*)programCount << std::endl; */

        result = loc->eval(frameBase, moduleBase, unitBase, programCount);
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
