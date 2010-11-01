#ifndef BASE_TYPE_H__A6694601_D86B_47DB_A798_067A5965ED0F
#define BASE_TYPE_H__A6694601_D86B_47DB_A798_067A5965ED0F
//
// $Id: base_type.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <dwarf.h>
#include "interface.h"
#include "type.h"

namespace Dwarf
{
    /**
     * Wrapper for a Dwarf_Die of the DW_TAG_base_type type
     */
    CLASS  BaseType : public Type
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = DW_TAG_base_type } ;

        enum Encoding
        {
            e_none,
            e_address = DW_ATE_address,
            e_boolean = DW_ATE_boolean,
            e_complex_float = DW_ATE_complex_float,
            e_float = DW_ATE_float,
            e_signed = DW_ATE_signed,
            e_signed_char = DW_ATE_signed_char,
            e_unsigned = DW_ATE_unsigned,
            e_unsigned_char = DW_ATE_unsigned_char,
            e_imaginary_float = DW_ATE_imaginary_float,
            e_complex_int = 0x80,

            e_lo_user = DW_ATE_lo_user,
            e_hi_user = DW_ATE_hi_user,
        };

        /* how the base type is to be interpreted */
        Encoding encoding() const;

    protected:
        BaseType(Dwarf_Debug, Dwarf_Die);
    };
}
#endif // BASE_TYPE_H__A6694601_D86B_47DB_A798_067A5965ED0F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
