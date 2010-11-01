//
// $Id: volatile_type.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <dwarf.h>
#include <string.h>
#include "impl.h"
#include "volatile_type.h"

using namespace Dwarf;


VolatileType::VolatileType(Dwarf_Debug dbg, Dwarf_Die die)
    : DecoratedType(dbg, die)
{
}


char* VolatileType::name_impl() const
{
    char* name = 0;
    size_t len = 0;

    boost::shared_ptr<Type> type = this->type();
    if (type)
    {
        static const char VOLATILE_STR[] = " volatile";
        static const size_t VOLATILE_LEN = sizeof(VOLATILE_STR) - 1;

        const char* typeName = type->name() ? type->name() : "<unnamed>";

        len = strlen(typeName);
        name = reinterpret_cast<char*>(malloc(len + VOLATILE_LEN + 1));

        if (name)
        {
            memcpy(name, typeName, len);
            memcpy(name + len, VOLATILE_STR, VOLATILE_LEN);

            name[len + VOLATILE_LEN] = 0;
        }
    }

    return name;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
