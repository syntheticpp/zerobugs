#ifndef MEMBER_H__E101AC53_A24E_41E9_A813_04E1A5DCA65D
#define MEMBER_H__E101AC53_A24E_41E9_A813_04E1A5DCA65D
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
#include "aggregation.h"
#include "child.h"
#include "datum.h"
#include "variable.h"
#include "interface.h"

namespace Dwarf
{
    class StructTypeBase;

    template<typename> class IterationTraits;

    /**
     * Represents the member of a struct, class or union
     */
    CLASS DataMember : public Aggregation
                     , public Child<StructTypeBase>
    {
    public:
        enum { TAG = DW_TAG_member };

        // friend class IterationTraits<DataMember>;

        DataMember(Dwarf_Debug, Dwarf_Die);

        Dwarf_Unsigned byte_size() const;

        Dwarf_Unsigned bit_size() const;

        Dwarf_Off bit_offset() const;
        
        // for static member data
        std::shared_ptr<ConstValue> const_value() const;
        RefPtr<SharedString> linkage_name() const;
        // TODO
        // declaration
        // visibility

    protected:
        mutable RefPtr<SharedString> linkageName_; // mangled symbol name
    };


    typedef VariableT<StructTypeBase> StaticMember;
}
#endif // MEMBER_H__E101AC53_A24E_41E9_A813_04E1A5DCA65D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
