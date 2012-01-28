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

#include "private/generic_attr.h"
#include "utils.h"
#include "impl.h"
#include "subrange_type.h"

using namespace Dwarf;


SubrangeType::SubrangeType(Dwarf_Debug dbg, Dwarf_Die die)
    : Dimension(dbg, die)
{
}


Dwarf_Signed SubrangeType::lower_bound() const
{
    GenericAttr<DW_AT_lower_bound, Dwarf_Signed> attr(dbg(), die());
    return attr.is_null() ? 0 : attr.value();
}


Dwarf_Signed SubrangeType::upper_bound() const
{
    GenericAttr<DW_AT_upper_bound, Dwarf_Signed> attr(dbg(), die());
    return attr.is_null() ? 0 : attr.value();
}


Dwarf_Unsigned SubrangeType::count() const
{
    GenericAttr<DW_AT_count, Dwarf_Unsigned> attr(dbg(), die());
    return attr.is_null() ? 0 : attr.value();
}


Dwarf_Unsigned SubrangeType::size() const
{
    Dwarf_Unsigned result = count();

    if (!result)
    {
        result = upper_bound() - lower_bound();
        if (result)
        {
            ++result;
        }
    }

    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
