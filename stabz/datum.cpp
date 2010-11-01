// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: datum.cpp 714 2010-10-17 10:03:52Z root $
//
#include <cassert>
#include "public/datum.h"
#include "public/fwdtype.h"

Stab::Datum::Datum(SharedString& name, DataType& type)
    : name_(&name)
    , type_(&type)
{
}


SharedString& Stab::Datum::name() const
{
    assert(!name_.is_null());
    return *name_;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
