#ifndef COUNT_ATTR_H__74BB6FB6_81ED_4235_8671_184C32D4D1F7
#define COUNT_ATTR_H__74BB6FB6_81ED_4235_8671_184C32D4D1F7
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
    /* Wrapper for DW_AT_count */
    class CountAttr : public Attribute
    {
        friend class SubrangeType;

    public:
        Dwarf_Unsigned value() const;

    protected:
        CountAttr(Dwarf_Debug, Dwarf_Die);
    };
}

#endif // COUNT_ATTR_H__74BB6FB6_81ED_4235_8671_184C32D4D1F7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
