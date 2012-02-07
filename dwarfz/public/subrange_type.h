#ifndef SUBRANGE_TYPE_H__39C876BA_A5A8_49F2_B188_664B101EF4A2
#define SUBRANGE_TYPE_H__39C876BA_A5A8_49F2_B188_664B101EF4A2
//
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
//
#include <dwarf.h>
#include "interface.h"
#include "dimension.h"

namespace Dwarf
{
    template<typename T> class IterationTraits;


    /**
     * Wrapper for DW_TAG_subrange_type
     */
    CLASS SubrangeType : public Dimension
    {
        DECLARE_CONST_VISITABLE()
        friend class IterationTraits<SubrangeType>;

    public:
        enum { TAG = DW_TAG_subrange_type };

        SubrangeType(Dwarf_Debug, Dwarf_Die);

        Dwarf_Unsigned size() const;

        Dwarf_Signed lower_bound() const;
        Dwarf_Signed upper_bound() const;
        Dwarf_Unsigned count() const;
    };
}

#endif // SUBRANGE_TYPE_H__39C876BA_A5A8_49F2_B188_664B101EF4A2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
