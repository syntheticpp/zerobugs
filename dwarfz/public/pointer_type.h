#ifndef POINTER_TYPE_H__EEBAC84E_38AC_400C_A1CC_7B2E96F22886
#define POINTER_TYPE_H__EEBAC84E_38AC_400C_A1CC_7B2E96F22886
//
// $Id: pointer_type.h 714 2010-10-17 10:03:52Z root $
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
#include "decorated_type.h"
#include "interface.h"

namespace Dwarf
{
    /**
     * Wrapper for DW_TAG_pointer_type die entries
     */
    CLASS PointerType : public DecoratedType
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = DW_TAG_pointer_type };

    protected:
        PointerType(Dwarf_Debug, Dwarf_Die);

        bool is_pointer_or_ref() const { return true; }

    private:
        char* name_impl() const;
    };
}
#endif // POINTER_TYPE_H__EEBAC84E_38AC_400C_A1CC_7B2E96F22886
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
