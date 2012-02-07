#ifndef ENUMERATION_TYPE_H__73B0876C_5075_4539_9CE6_C7B326A95C1F
#define ENUMERATION_TYPE_H__73B0876C_5075_4539_9CE6_C7B326A95C1F
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
#include "child.h"
#include "dimension.h"
#include "list.h"

namespace Dwarf
{
    /**
     * Wrapper for DW_TAG_subrange_type
     */
    CLASS EnumType : public Dimension
    {
        DECLARE_CONST_VISITABLE()
        friend class IterationTraits<EnumType>;

    public:
        enum { TAG = DW_TAG_enumeration_type };

        EnumType(Dwarf_Debug, Dwarf_Die);

        Dwarf_Unsigned size() const;

        Dwarf_Signed lower_bound() const;
        Dwarf_Signed upper_bound() const;

        /* Wraps DW_TAG_enumerator */
        CLASS Enumerator : public Die, public Child<EnumType>
        {
        public:
            enum { TAG = DW_TAG_enumerator };

            Enumerator(Dwarf_Debug, Dwarf_Die);
            // friend class IterationTraits<Enumerator>;

            Dwarf_Signed value() const;
        };


        List<Enumerator> enums() const;
    };


    inline bool operator<(const EnumType::Enumerator& lhs,
                          const EnumType::Enumerator& rhs)
    {
        return lhs.value() < rhs.value();
    }
}

#endif // ENUMERATION_TYPE_H__73B0876C_5075_4539_9CE6_C7B326A95C1F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
