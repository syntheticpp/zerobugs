#ifndef PTR_TO_MEMBER_TYPE_H__99DE0B4C_E985_4B6F_9677_D26F0A074BE4
#define PTR_TO_MEMBER_TYPE_H__99DE0B4C_E985_4B6F_9677_D26F0A074BE4
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
#include "type.h"

namespace Dwarf
{
    class Location;


    CLASS PtrToMemberType : public Type
    {
    public:
        enum { TAG = DW_TAG_ptr_to_member_type };

        DECLARE_CONST_VISITABLE()

        ~PtrToMemberType() throw()
        {
        }

        boost::shared_ptr<Type> type() const;
        boost::shared_ptr<Type> containing_type() const;

        bool is_pointer_or_ref() const { return true; }

    protected:
        PtrToMemberType(Dwarf_Debug dbg, Dwarf_Die die) : Type(dbg, die)
        {
        }

    private:
        mutable boost::shared_ptr<Type> type_;
        mutable boost::shared_ptr<Type> containingType_;
    };
};
#endif // PTR_TO_MEMBER_TYPE_H__99DE0B4C_E985_4B6F_9677_D26F0A074BE4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
