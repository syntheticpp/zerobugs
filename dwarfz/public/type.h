#ifndef TYPE_H__14EA6F92_C3E9_4113_A540_CD0368C029B2
#define TYPE_H__14EA6F92_C3E9_4113_A540_CD0368C029B2
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
#include "die.h"
#include "generic/visitor.h"
#include "iterator.h"
#include "list.h"
#include "template_type.h"
#include "interface.h"


namespace Dwarf
{
    class CompileUnit;

    /**
     * Intermediary class that serves as a base
     * to other classes that represent data types
     * (such as BaseType, PointerType, etc.)
     */
    CLASS Type : public Die, public ConstVisitable<>
    {
    public:
        typedef CompileUnit parent_type;
        friend class CompileUnit;
        friend class IterationTraits<Type>;

        ~Type() throw();

        /**
         * @return the size of this type, in bits
         */
        Dwarf_Unsigned bit_size() const;

        /**
         * @return the size of this type in bytes
         */
        Dwarf_Unsigned byte_size() const;

        bool is_incomplete() const;

        bool is_complete() const { return !is_incomplete(); }

        Dwarf_Off decl_offset() const;

        std::shared_ptr<Type> declaration() const;

        bool is_declaration() const
        {
            return Utils::has_attr(dbg(), die(), DW_AT_declaration);
        }

        bool is_specification() const
        {
            return Utils::has_attr(dbg(), die(), DW_AT_specification);
        }

        bool is_artificial() const;

        virtual bool is_pointer_or_ref() const { return false; }

        List<TemplateType<Type> > template_types() const;

    protected:
        Type(Dwarf_Debug dbg, Dwarf_Die die) : Die(dbg, die)
        {
        }

        char* name_impl() const;
    };


    template<> struct IterationTraits<Type>
    {
        typedef std::shared_ptr<Type> ptr_type;

        /**
         * Obtain the first element in the list
         */
        static ptr_type first(Dwarf_Debug dbg, Dwarf_Die die);

        /**
         * Get the sibling of same type for a given element
         */
        static void next(ptr_type& elem);
    };
}
#endif // TYPE_H__14EA6F92_C3E9_4113_A540_CD0368C029B2
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
