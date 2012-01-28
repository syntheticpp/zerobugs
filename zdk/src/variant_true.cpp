// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "zdk/variant_util.h"

bool variant_true(const Variant& var)
{
    bool result = false;

    if (is_integer(var))
    {
        result = (var.bits() != 0);
    }
    else if (is_float(var))
    {
        result = (var.long_double() != 0);
    }
    else if (var.type_tag() == Variant::VT_POINTER)
    {
        result = (var.pointer() != 0);
    }
    else if (var.type_tag() == Variant::VT_OBJECT
          || var.type_tag() == Variant::VT_ARRAY)
    {
        result = true;
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
