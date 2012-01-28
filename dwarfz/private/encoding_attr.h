#ifndef ENCODING_ATTR_H__FF1B8C87_F85F_4B93_8EB4_979141BADF61
#define ENCODING_ATTR_H__FF1B8C87_F85F_4B93_8EB4_979141BADF61
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
    class BaseType;

    /* Wrapper for DW_AT_encoding */
    class EncodingAttr : public Attribute
    {
        friend class BaseType;

    public:
        // TODO: return enumerated type
        Dwarf_Unsigned value() const;

    protected:
        EncodingAttr(Dwarf_Debug, Dwarf_Die);
    };
}

#endif // ENCODING_ATTR_H__FF1B8C87_F85F_4B93_8EB4_979141BADF61
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
