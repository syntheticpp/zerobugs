#ifndef CLASS_TYPE_H__9BC55163_79C8_4C54_BB64_2003BE33D6CF
#define CLASS_TYPE_H__9BC55163_79C8_4C54_BB64_2003BE33D6CF
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

#include <memory>
#include <vector>
#include <dwarf.h>
#include "function.h"
#include "interface.h"
#include "list.h"
#include "member.h"
#include "type.h"


namespace Dwarf
{
    class Inheritance;
    class KlassType;


    CLASS StructTypeBase : public Type
    {
    public:
        typedef std::vector<std::shared_ptr<StaticMember> > StaticMemData;

        /**
         * @return the member data
         */
        List<DataMember> members() const;

        const StaticMemData& static_members() const;

    protected:
        StructTypeBase(Dwarf_Debug, Dwarf_Die);

        virtual ~StructTypeBase() throw() {}

    private:
        mutable std::auto_ptr<StaticMemData> staticMembers_;
    };


    /**
     * Member function (aka method) meta info
     */
    CLASS MemFun : public Function
    {
        // friend class IterationTraits<MemFun>;

    public:
        typedef KlassType parent_type;

        bool is_virtual() const;

        // Access access() const;

        Dwarf_Addr vtable_offset(
            Dwarf_Addr frameBase,
            Dwarf_Addr moduleBase,
            Dwarf_Addr unitBase,
            Dwarf_Addr programCount) const;

        MemFun(Dwarf_Debug dbg, Dwarf_Die die) : Function(dbg, die)
        {
        }
    };


    /**
     * Wrapper for:
     * DW_TAG_class_type (template instantiations)
     * DW_TAG_structure_type
     */
    CLASS KlassType : public StructTypeBase
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = DW_TAG_structure_type };

        typedef std::vector<std::shared_ptr<Inheritance> > BaseList;


        virtual ~KlassType() throw() {}

        /**
         * Return the base classes
         */
        const BaseList& bases() const;

        const MethodList& methods() const;

    protected:
        KlassType(Dwarf_Debug, Dwarf_Die);

    private:
        mutable std::auto_ptr<MethodList> methods_;
        mutable std::auto_ptr<BaseList> bases_;
    };


    /**
     * Wrapper for DW_TAG_union_type
     */
    CLASS UnionType : public StructTypeBase
    {
        DECLARE_CONST_VISITABLE()

    public:
        enum { TAG = DW_TAG_union_type };

    protected:
        UnionType(Dwarf_Debug, Dwarf_Die);
    };
}

#endif // CLASS_TYPE_H__9BC55163_79C8_4C54_BB64_2003BE33D6CF
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
