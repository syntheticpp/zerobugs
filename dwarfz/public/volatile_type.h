#ifndef VOLATILE_TYPE_H__7510242F_C481_4A67_9A60_1932C6408D8D
#define VOLATILE_TYPE_H__7510242F_C481_4A67_9A60_1932C6408D8D
//
//
// $Id: volatile_type.h 714 2010-10-17 10:03:52Z root $
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

#include "interface.h"
#include "decorated_type.h"

namespace Dwarf
{
    /**
     * Models a volatile-qualified type
     */
    CLASS VolatileType : public DecoratedType
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = DW_TAG_volatile_type };

    protected:
        VolatileType(Dwarf_Debug, Dwarf_Die);

        char* name_impl() const;
    };
}

#endif // VOLATILE_TYPE_H__7510242F_C481_4A67_9A60_1932C6408D8D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
