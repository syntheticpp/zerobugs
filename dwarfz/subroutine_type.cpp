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
#include "subroutine_type.h"

using namespace Dwarf;


SubroutineType::SubroutineType(Dwarf_Debug dbg, Dwarf_Die die)
    : Type(dbg, die)
{
}


List<Parameter> SubroutineType::params() const
{
    return List<Param>(this->dbg(), this->die());
}


boost::shared_ptr<Type> SubroutineType::ret_type() const
{
    return Utils::type(dbg(), die());
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
