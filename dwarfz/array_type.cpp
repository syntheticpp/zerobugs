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

#include <string>
#include "utils.h"
#include "enum_type.h"
#include "subrange_type.h"
#include "impl.h"
#include "array_type.h"

using namespace Dwarf;


ArrayType::ArrayType(Dwarf_Debug dbg, Dwarf_Die die)
    : Type(dbg, die)
{
    assert(Utils::tag(dbg, die) == TAG);
}


boost::shared_ptr<Type> ArrayType::elem_type() const
{
    return Utils::type(this->dbg(), this->die());
}


/*
char* ArrayType::name_impl() const
{
    std::string name;

    boost::shared_ptr<Type> type = elem_type();
    if (type)
    {
        name = string(type->name()) + "[]";
    }
    return strdup(name.c_str());
}
*/


List<Dimension> ArrayType::dimensions() const
{
    return List<Dimension>(dbg(), die());
}


boost::shared_ptr<Dimension>
IterationTraits<Dimension>::first(Dwarf_Debug dbg, Dwarf_Die die)
{
    boost::shared_ptr<Dimension> p =
        IterationTraits<SubrangeType>::first(dbg, die);
    if (!p)
    {
        p = IterationTraits<EnumType>::first(dbg, die);
    }
    return p;
}


/**
 * Get the sibling of same type for a given element
 */
void
IterationTraits<Dimension>::next(boost::shared_ptr<Dimension>& elem)
{
    assert(elem);

    // save elem value
    boost::shared_ptr<Dimension> tmp = elem;

    IterationTraits<EnumType>::next(elem);
    if (!elem)
    {
        elem = tmp;
        IterationTraits<SubrangeType>::next(elem);
    }
}


////////////////////////////////////////////////////////////////
// D Language Arrays
//
DynArrayType::DynArrayType(Dwarf_Debug dbg, Dwarf_Die die)
    : Type(dbg, die)
{
    assert(Utils::tag(dbg, die) == TAG);
}


boost::shared_ptr<Type> DynArrayType::elem_type() const
{
    return Utils::type(this->dbg(), this->die());
}


AssocArrayType::AssocArrayType(Dwarf_Debug dbg, Dwarf_Die die)
    : Type(dbg, die)
{
    assert(Utils::tag(dbg, die) == TAG);
}


boost::shared_ptr<Type> AssocArrayType::elem_type() const
{
    return Utils::type(this->dbg(), this->die());
}


/*
from Walter Bright <walter@digitalmars.com>
  to Cristian Vlasceanu <cristian@zerobugs.org>,
date        Apr 5, 2007 1:46 PM
subject     Re: DWARF representation of D arrays
    I've added Jasche to this, as he's faced with the same issues with the
    Codeview debugger.

    1) I think we can repurpose DW_AT_containing_type to be the key type,
    and leave DW_AT_data_type to be the element type.
    2) They're not contiguous. The file phobos/internal/aaA.d contains the
    structure (it's a hashed array of binary trees). The implementation (and
    therefore layout) is meant to be pluggable, which doesn't do a fixed
    debugger much good, but at least you can display the type correctly. Or,
    just hook the functions in aaA.d to retrieve the information.
 */
boost::shared_ptr<Type> AssocArrayType::key_type() const
{
    return Utils::containing_type(this->dbg(), this->die());
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
