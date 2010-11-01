//
// $Id: attr.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include "error.h"
#include "utils.h"
#include "impl.h"
#include "attr.h"

using namespace Dwarf;


Attribute::Attribute(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Half type)
    : dbg_(dbg), attr_(0), form_(0)
{
    assert(dbg);
    assert(die);

    Dwarf_Error err = 0;

    int res = dwarf_attr(die, type, &attr_, &err);
    if (res == DW_DLV_ERROR)
    {
        throw Error("Attribute::Attribute", dbg, err);
    }
    else if (res == DW_DLV_NO_ENTRY)
    {
        assert(err == NULL);
        assert(is_null());
    }
    else
    {
        assert(Utils::has_attr(dbg, die, type));
    }
}


Attribute::~Attribute() throw()
{
    if (attr_)
    {
        dwarf_dealloc(dbg_, attr_, DW_DLA_ATTR);
    }
}


Dwarf_Half Attribute::form() const
{
    if (form_ == 0)
    {
        Dwarf_Error err = 0;

        if (dwarf_whatform_direct(attr_, &form_, &err) == DW_DLV_ERROR)
        {
            throw Error("Attribute::form", dbg(), err);
        }
    }
    return form_;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
