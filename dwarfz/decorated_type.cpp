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

#include "utils.h"
#include "impl.h"
#include "decorated_type.h"

using namespace std;
using namespace Dwarf;


DecoratedType::DecoratedType(Dwarf_Debug dbg, Dwarf_Die die)
    : Type(dbg, die)
{
}


boost::shared_ptr<Type> DecoratedType::type() const
{
    if (!type_)
    {
        type_ = Utils::type(dbg(), die());
    }
    return type_;
}


bool DecoratedType::is_pointer_or_ref() const
{
    if (!type_)
    {
        type_ = Utils::type(dbg(), die());
    }
    return type_ ? type_->is_pointer_or_ref() : false;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
