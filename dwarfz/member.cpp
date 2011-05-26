//
// $Id: member.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/shared_string_impl.h"
#include "attr.h"
#include "private/generic_attr.h"
#include "const_value.h"
#include "error.h"
#include "type.h"
#include "utils.h"
#include "impl.h"
#include "member.h"


using namespace std;
using namespace Dwarf;


DataMember::DataMember(Dwarf_Debug dbg, Dwarf_Die die)
    : Aggregation(dbg, die)
{
}


Dwarf_Unsigned DataMember::byte_size() const
{
    Dwarf_Unsigned size = Utils::byte_size(dbg(), die());
    return size;
}


Dwarf_Unsigned DataMember::bit_size() const
{
    Dwarf_Unsigned size = Utils::bit_size(dbg(), die());
    return size;
}


Dwarf_Off DataMember::bit_offset() const
{
    return Utils::bit_offset(dbg(), die());
}


RefPtr<SharedString> DataMember::linkage_name() const
{
    if (linkageName_.is_null())
    {
        string name;

        if (!Utils::get_linkage_name(dbg(), die(), name))
        {
            if (boost::shared_ptr<Die> tmp = check_indirect())
            {
                Utils::get_linkage_name(dbg(), tmp->die(), name);
            }
        }
        linkageName_ = shared_string(name);
    }
    return linkageName_;
}


boost::shared_ptr<ConstValue> DataMember::const_value() const
{
    boost::shared_ptr<ConstValue> val;
    if (Utils::has_attr(dbg(), die(), DW_AT_const_value))
    {
        GenericAttr<DW_AT_const_value, Dwarf_Unsigned> attr(dbg(), die());
        if (attr.is_block())
        {
            val.reset(new ConstValue(attr.block()));
        }
        else
        {
            val.reset(new ConstValue(attr.value()));
        }
    }
    return val;
}
// Copyright (c) 2004, 2005 Cristian L. Vlasceanu

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
