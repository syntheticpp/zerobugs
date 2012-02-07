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

#include <assert.h>
#include "access_attr.h"
#include "utils.h"
#include "impl.h"
#include "aggregation.h"

using namespace Dwarf;


Aggregation::Aggregation (Dwarf_Debug dbg, Dwarf_Die die)
    : Die(dbg, die)
{
}


Aggregation::~Aggregation() throw()
{
}


Access Aggregation::access() const
{
    AccessAttr attr(dbg(), die());

    if (!attr.is_null())
    {
        switch (attr.value())
        {
        case DW_ACCESS_public: return a_public;
        case DW_ACCESS_protected: return a_protected;
        case DW_ACCESS_private: return a_private;

        default: assert(false);
        }
    }
    return a_public;
}


std::shared_ptr<Location> Aggregation::loc() const
{
    return Utils::loc(dbg(), die(), DW_AT_data_member_location);
}


std::shared_ptr<Type> Aggregation::type() const
{
    if (!type_)
    {
        assert(!Utils::has_attr(this->dbg(), this->die(), DW_AT_abstract_origin));
        type_ = Utils::type(this->dbg(), this->die());
    }
    return type_;
}


// Copyright (c) 2004, 2006 Cristian L. Vlasceanu
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
