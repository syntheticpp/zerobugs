#ifndef INHERITANCE_H__50DC52A2_D35C_4644_B474_F8B6ED3CB59D
#define INHERITANCE_H__50DC52A2_D35C_4644_B474_F8B6ED3CB59D
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

#include "aggregation.h"
#include "child.h"
#include "interface.h"
#include "list.h"

namespace Dwarf
{
    class KlassType;


    /**
     * Wrapper for Dwarf_Die records of the
     * DW_TAG_inheritance type. An Inheritance
     * entry defines a base class or structure
     * and it is owned by the derived class or
     * structure.
     */
    CLASS Inheritance : public Aggregation, public Child<KlassType>
    {
    public:
        enum { TAG = DW_TAG_inheritance };

        friend class IterationTraits<Inheritance>;

        bool is_virtual() const;

    protected:
        Inheritance(Dwarf_Debug dbg, Dwarf_Die die);
    };
}

#endif // INHERITANCE_H__50DC52A2_D35C_4644_B474_F8B6ED3CB59D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
