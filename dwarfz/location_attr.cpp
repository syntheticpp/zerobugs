//
// $Id: location_attr.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include "location.h"
#include "impl.h"
#include "location_attr.h"
#include "utils.h"

using namespace Dwarf;

LocationAttr::LocationAttr(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr)
    : Attribute(dbg, die, attr)
{
}


LocationAttr::loc_ptr_type LocationAttr::value() const
{
    return loc_ptr_type(new Location(this->dbg(), this->attr()));
}


boost::shared_ptr<LocationAttr>
LocationAttr::create_instance(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half attr)
{
    boost::shared_ptr<LocationAttr> loc_attr;

/* todo: use a recursive template here instead of the switch? */
    switch (attr)
    {
    case DW_AT_location:
        loc_attr.reset(new LocationAttrT<DW_AT_location>(dbg, die));
        break;

    case DW_AT_frame_base:
        loc_attr.reset(new LocationAttrT<DW_AT_frame_base>(dbg, die));
        break;

    case DW_AT_data_member_location:
        loc_attr.reset(
            new LocationAttrT<DW_AT_data_member_location>(dbg, die));
        break;

    case DW_AT_data_location:
        loc_attr.reset(new LocationAttrT<DW_AT_data_location>(dbg, die));
        break;

    case DW_AT_vtable_elem_location:
        loc_attr.reset(new LocationAttrT<DW_AT_vtable_elem_location>(dbg, die));
        break;

    case DW_AT_use_location:
        assert(Utils::tag(dbg, die) == DW_TAG_ptr_to_member_type);
        loc_attr.reset(new LocationAttrT<DW_AT_use_location>(dbg, die));
        break;

    default: assert(false);
    }

    if (loc_attr->is_null())
    {
        loc_attr.reset();
    }
    return loc_attr;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
