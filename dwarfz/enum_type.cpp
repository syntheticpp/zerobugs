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

#include <algorithm>    // min_element, max_element

#include "private/generic_attr.h"
#include "impl.h"
#include "enum_type.h"

using namespace Dwarf;


EnumType::EnumType(Dwarf_Debug dbg, Dwarf_Die die)
    : Dimension(dbg, die)
{
}


List<EnumType::Enumerator> EnumType::enums() const
{
    return List<Enumerator>(dbg(), die());
}


Dwarf_Signed EnumType::lower_bound() const
{
    List<Enumerator> enums(this->enums());
    return std::min_element(enums.begin(), enums.end())->value();
}


Dwarf_Signed EnumType::upper_bound() const
{
    List<Enumerator> enums(this->enums());
    return std::max_element(enums.begin(), enums.end())->value();
}


Dwarf_Unsigned EnumType::size() const
{
    return enums().size();
}


EnumType::Enumerator::Enumerator(Dwarf_Debug dbg, Dwarf_Die die)
    : Die(dbg, die)
{
}


Dwarf_Signed EnumType::Enumerator::value() const
{
    GenericAttr<DW_AT_const_value, Dwarf_Signed> attr(dbg(), die());
    return attr.is_null() ? 0 : attr.value();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
