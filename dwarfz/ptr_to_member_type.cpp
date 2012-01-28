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

#include "ptr_to_member_type.h"
#include "utils.h"

using namespace Dwarf;

boost::shared_ptr<Type> PtrToMemberType::type() const
{
    if (!type_)
    {
        type_ = Utils::type(dbg(), die());
    }
    return type_;
}


boost::shared_ptr<Type> PtrToMemberType::containing_type() const
{
    if (!containingType_)
    {
        containingType_ = Utils::containing_type(dbg(), die());
    }
    return containingType_;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
