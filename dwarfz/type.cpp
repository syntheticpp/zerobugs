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
//
#include <dwarf.h>
#include "debug.h"
#include "generic_attr.h"
#include "indirect.h"
#include "utils.h"
#include "impl.h"
#include "type.h"

using namespace Dwarf;
using namespace std;


Type::~Type() throw()
{
}


Dwarf_Unsigned Type::bit_size() const
{
    return Utils::bit_size(dbg(), die());
}


Dwarf_Unsigned Type::byte_size() const
{
    return Utils::byte_size(dbg(), die());
}


char* Type::name_impl() const
{
    char* name = Die::name_impl();
    if (!name)
    {
        boost::shared_ptr<Die> indir = check_indirect();
        if (indir)
        {
            name = strdup(indir->name());
        }
    }
    return name;
}


bool Type::is_incomplete() const
{
    return Utils::has_attr(dbg(), die(), DW_AT_declaration)
        &&!Utils::has_attr(dbg(), die(), DW_AT_specification);
}


Dwarf_Off Type::decl_offset() const
{
    typedef GenericAttr<DW_AT_specification, Dwarf_Off> SpecAttr;
    return is_specification() ? SpecAttr(dbg(), die()).value() : 0;
}


bool Type::is_artificial() const
{
    if (Utils::has_attr(dbg(), die(), DW_AT_artificial))
    {
        return true;
    }
    return false;
}


List<TemplateType<Type> > Type::template_types() const
{
    return List<TemplateType<Type> >(dbg(), die());
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
