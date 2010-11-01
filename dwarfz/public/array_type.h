#ifndef ARRAY_TYPE_H__39611207_B312_45CB_96C9_B8F6AF991650
#define ARRAY_TYPE_H__39611207_B312_45CB_96C9_B8F6AF991650
//
// $Id: array_type.h 714 2010-10-17 10:03:52Z root $
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
#include "dimension.h"
#include "list.h"

namespace Dwarf
{
    /**
     * Wrapper for Dwarf_Die entries of type DW_TAG_array_type
     */
    CLASS ArrayType : public Type
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = DW_TAG_array_type };

        /**
         * The type of an array element
         */
        boost::shared_ptr<Type> elem_type() const;

        /**
         * The total size in bytes of an array
         * instance of this type
         */
        Dwarf_Unsigned byte_size() const;

        List<Dimension> dimensions() const;

    protected:
        ArrayType(Dwarf_Debug, Dwarf_Die);
    };


    template<> struct IterationTraits<Dimension>
    {
        typedef boost::shared_ptr<Dimension> ptr_type;

        /**
         * Obtain the first element in the list
         */
        static ptr_type first(Dwarf_Debug dbg, Dwarf_Die die);

        /**
         * Get the sibling of same type for a given element
         */
        static void next(ptr_type& elem);
    };


    /**
     * D Language  Dynamic Array
     */
    CLASS DynArrayType : public Type
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = 0x41 };

        /**
         * The type of an array element
         */
        boost::shared_ptr<Type> elem_type() const;

    protected:
        DynArrayType(Dwarf_Debug, Dwarf_Die);
    };

    /**
     * D Language   Associative Array
     */
    CLASS AssocArrayType : public Type
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = 0x42 };

        boost::shared_ptr<Type> elem_type() const;

        boost::shared_ptr<Type> key_type() const;

    protected:
        AssocArrayType(Dwarf_Debug, Dwarf_Die);
    };
}
#endif // ARRAY_TYPE_H__39611207_B312_45CB_96C9_B8F6AF991650

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
