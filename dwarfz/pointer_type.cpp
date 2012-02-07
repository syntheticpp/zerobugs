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
#include <string.h>
#include "impl.h"
#include "public/pointer_type.h"


using namespace Dwarf;


PointerType::PointerType(Dwarf_Debug dbg, Dwarf_Die die)
    : DecoratedType(dbg, die)
{
}


char* PointerType::name_impl() const
{
    char* name = 0;
    size_t len = 0;

    std::shared_ptr<Type> type = this->type();
    if (type)
    {
        const char* typeName = type->name() ? type->name() : "<unnamed>";

        len = strlen(typeName);
        name = reinterpret_cast<char*>(malloc(len + 2));

        strcpy(name, typeName);
    }

    if (name)
    {
        switch (get_tag())
        {
        case DW_TAG_pointer_type:
            name[len++] = '*';
            break;

        case DW_TAG_reference_type:
            name[len++] = '&';
            break;

        default:
            assert(false);
        }
        name[len] = 0;
    }
    return name;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
