#ifndef TYPE_ATTR_H__F1DEA043_3B60_4161_82C7_33B9CC4011C1
#define TYPE_ATTR_H__F1DEA043_3B60_4161_82C7_33B9CC4011C1
//
// $Id: type_attr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/shared_ptr.hpp>
#include "attr.h"
#include "interface.h"

namespace Dwarf
{
    class Type;


    /**
     * Wrapper for Dwarf_Attribute of the DW_AT_type type
     */
    class TypeAttr : public Attribute
    {
    public:
        friend class Utils;

        boost::shared_ptr<Type> value() const;

    protected:
        TypeAttr(Dwarf_Debug, Dwarf_Die, Dwarf_Half = DW_AT_type);
    };
}
#endif // TYPE_ATTR_H__F1DEA043_3B60_4161_82C7_33B9CC4011C1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
