#ifndef ACCESS_ATTR_H__EE4081F6_192F_43F6_84AB_98346264C350
#define ACCESS_ATTR_H__EE4081F6_192F_43F6_84AB_98346264C350
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

#include "attr.h"
#include "interface.h"

namespace Dwarf
{
    /* Wrapper for DW_AT_accessibility */
    class AccessAttr : public Attribute
    {
        friend class Aggregation;

    public:
        // TODO: return an Access enumerated type
        Dwarf_Unsigned value() const;

    protected:
        AccessAttr(Dwarf_Debug, Dwarf_Die);
    };
}

#endif // ACCESS_ATTR_H__EE4081F6_192F_43F6_84AB_98346264C350
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
