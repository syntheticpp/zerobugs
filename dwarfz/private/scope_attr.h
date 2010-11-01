#ifndef SCOPE_ATTR_H__E5D75FFE_E2C3_4F55_B757_ABA8D79CCD9D
#define SCOPE_ATTR_H__E5D75FFE_E2C3_4F55_B757_ABA8D79CCD9D
//
// $Id: scope_attr.h 714 2010-10-17 10:03:52Z root $
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
    class StartScopeAttr : public Attribute
    {
        friend class Datum;

    public:
        /**
         * @return offset in bytes from the
         * beginning of the enclosing scope
         */
        Dwarf_Off byte_offset() const;

    protected:
        StartScopeAttr(Dwarf_Debug, Dwarf_Die);
    };
}

#endif // SCOPE_ATTR_H__E5D75FFE_E2C3_4F55_B757_ABA8D79CCD9D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
