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

#include "impl.h"
#include "typedef.h"

using namespace Dwarf;


Typedef::Typedef(Dwarf_Debug dbg, Dwarf_Die die)
    : DecoratedType(dbg, die)
{
}


boost::shared_ptr<Type> Typedef::type() const
{
    boost::shared_ptr<Type> tp = DecoratedType::type();
    if (tp)
    {
        const char* name = tp->name();
        //
        // handle typedef-ed unnamed structs, unions, and C++ classes
        //
        if (!name || !name[0])
        {
            set_name(*tp, this->name());
        }
        assert(tp->name());
        assert(tp->name()[0]);
    }
    return tp;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
