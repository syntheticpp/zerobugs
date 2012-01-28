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
#include "private/generic_attr.h"
#include "type.h"

#include "impl.h"
#include "inheritance.h"

using namespace Dwarf;

Inheritance::Inheritance(Dwarf_Debug dbg, Dwarf_Die die)
    : Aggregation(dbg, die)
{
}


bool Inheritance::is_virtual() const
{
    GenericAttr<DW_AT_virtuality, bool> attr(dbg(), die());
    return attr.is_null() ? false : attr.value();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
