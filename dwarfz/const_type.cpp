//
// $Id: const_type.cpp 713 2010-10-16 07:10:27Z root $
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
#include "const_type.h"

//using namespace std;
using namespace Dwarf;


ConstType::ConstType(Dwarf_Debug dbg, Dwarf_Die die)
    : DecoratedType(dbg, die)
{
}


char* ConstType::name_impl() const
{
    char* name = 0;
    size_t len = 0;

    boost::shared_ptr<Type> type = this->type();
    if (type)
    {
        static const char CONST_STR[] = " const";
        static const size_t CONST_LEN = sizeof(CONST_STR) - 1;

        assert(CONST_LEN == 6);
        const char* typeName = type->name() ? type->name() : "<unnamed>";

        len = strlen(typeName);
        name = reinterpret_cast<char*>(malloc(len + CONST_LEN + 1));

        if (name)
        {
            memcpy(name, typeName, len);
            memcpy(name + len, CONST_STR, CONST_LEN);

            name[len + CONST_LEN] = 0;
        }
    }

    return name;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
