#ifndef CONST_TYPE_H__B87F1A37_AD0A_4D4D_8932_CEE22D21B346
#define CONST_TYPE_H__B87F1A37_AD0A_4D4D_8932_CEE22D21B346
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

#include <dwarf.h>
#include "interface.h"
#include "decorated_type.h"

namespace Dwarf
{
    /**
     * A const-qualified type
     */
    CLASS ConstType : public DecoratedType
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = DW_TAG_const_type };

    protected:
        ConstType(Dwarf_Debug, Dwarf_Die);

        char* name_impl() const;
    };
}
#endif // CONST_TYPE_H__B87F1A37_AD0A_4D4D_8932_CEE22D21B346
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
