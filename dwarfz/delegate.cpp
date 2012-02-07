//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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

#include "delegate.h"
#include "utils.h"

using namespace Dwarf;


Delegate::Delegate(Dwarf_Debug dbg, Dwarf_Die die) : Type(dbg, die)
{
}


std::shared_ptr<Type> Delegate::function_type() const
{
    return Utils::type(dbg(), die());
}


std::shared_ptr<Type> Delegate::this_type() const
{
    return Utils::containing_type(dbg(), die());
}
