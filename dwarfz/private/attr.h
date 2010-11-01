#ifndef ATTR_H__D5681F86_0F32_4A50_8F04_F051D733ACD3
#define ATTR_H__D5681F86_0F32_4A50_8F04_F051D733ACD3
//
// $Id: attr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <boost/utility.hpp>
#include <dwarf.h>
#include <libelf.h>
#include <libdwarf.h>
#include "dwarfz/public/interface.h"

namespace Dwarf
{
    class Attribute : boost::noncopyable
    {
    public:
        virtual ~Attribute() throw();

        bool is_null() const { return attr_ == 0; }

        bool is_block() const
        {
            return form() == DW_FORM_block
                || form() == DW_FORM_block1
                || form() == DW_FORM_block2
                || form() == DW_FORM_block4;
        }

    protected:
        Attribute(Dwarf_Debug, Dwarf_Die, Dwarf_Half);

        Dwarf_Debug dbg() const { return dbg_; }

        Dwarf_Attribute attr() const { return attr_; }
        Dwarf_Half form() const;

    private:
        Dwarf_Debug dbg_;
        Dwarf_Attribute attr_;
        mutable Dwarf_Half form_;
    };
}
#endif // ATTR_H__D5681F86_0F32_4A50_8F04_F051D733ACD3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
