#ifndef DIMENSION_H__438E6B8D_A459_4D94_AAD5_DD213653CE31
#define DIMENSION_H__438E6B8D_A459_4D94_AAD5_DD213653CE31
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

#include "type.h"

namespace Dwarf
{
    class ArrayType;
    class EnumType;
    class SubrangeType;

    template<typename T> class IterationTraits;


    /**
     * Base for SubrangeType and EnumerationType
     * @note used for array sizes
     * @see Dwarf::ArrayType
     */
    CLASS Dimension : public Type
    {
        friend class IterationTraits<EnumType>;
        friend class IterationTraits<SubrangeType>;

    public:
        typedef ArrayType parent_type;

        virtual Dwarf_Unsigned size() const = 0;
        virtual Dwarf_Signed lower_bound() const = 0;
        virtual Dwarf_Signed upper_bound() const = 0;

    protected:
        Dimension(Dwarf_Debug dbg, Dwarf_Die die) : Type(dbg, die)
        {}
    };
}

#endif // DIMENSION_H__438E6B8D_A459_4D94_AAD5_DD213653CE31
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
