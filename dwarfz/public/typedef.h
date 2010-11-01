#ifndef TYPEDEF_H__B87F1A37_AD0A_4D4D_8932_CEE22D21B346
#define TYPEDEF_H__B87F1A37_AD0A_4D4D_8932_CEE22D21B346
//
//
// $Id: typedef.h 714 2010-10-17 10:03:52Z root $
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
     * Introduces a new name for an existent type -- models
     * the C and C++ "typedef" language feature.
     */
    CLASS Typedef : public DecoratedType
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = DW_TAG_typedef };

        virtual boost::shared_ptr<Type> type() const;

    protected:
        Typedef(Dwarf_Debug, Dwarf_Die);
    };
}
#endif // TYPEDEF_H__B87F1A37_AD0A_4D4D_8932_CEE22D21B346
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
