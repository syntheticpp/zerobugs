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

#include <iostream>
#include "public/debug.h"
#include "public/imported_decl.h"
#include "public/utils.h"
#include "private/generic_attr.h"


using namespace Dwarf;


ImportedDecl::ImportedDecl(Dwarf_Debug dbg, Dwarf_Die die) : Die(dbg, die)
{
}


boost::shared_ptr<Die> ImportedDecl::get_import()
{
    boost::shared_ptr<Die> result;
    if (Utils::has_attr(dbg(), die(), DW_AT_import))
    {
        Dwarf::GenericAttr<DW_AT_import, Dwarf_Off> attr(dbg(), die());
        //result = owner().get_object(attr.value(), false);
        result = owner().get_object(attr.value(), true);
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
