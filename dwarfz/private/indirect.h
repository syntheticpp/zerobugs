#ifndef INDIRECT_H__D9254AA9_A04D_413C_B271_2B63C6043D8F
#define INDIRECT_H__D9254AA9_A04D_413C_B271_2B63C6043D8F
//
// $Id: indirect.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "debug.h"

template<Dwarf_Half A>
boost::shared_ptr<Dwarf::Die> get_indirect(
    Dwarf_Debug         dbg,
    Dwarf_Die           die,
    const Dwarf::Debug* owner = NULL)
{
    boost::shared_ptr<Dwarf::Die> result;
    Dwarf::GenericAttr<A, Dwarf_Off> attr(dbg, die);

    if (!attr.is_null())
    {
        if (owner == 0)
        {
            owner = Dwarf::Debug::get_wrapper(dbg);
        }
        if (owner)
        {
            const Dwarf_Off offs = attr.value();
            result = owner->get_object(offs, false, false);
        }
    }
    return result;
}
#endif // INDIRECT_H__D9254AA9_A04D_413C_B271_2B63C6043D8F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
