#ifndef PRODUCER_H__81BC708E_A617_4016_A149_8150BE47F425
#define PRODUCER_H__81BC708E_A617_4016_A149_8150BE47F425
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
#include "die.h"

namespace Dwarf
{
    void register_creator(
        Dwarf_Half,
        std::shared_ptr<Die> (*)(Dwarf_Debug, Dwarf_Die));

    /**
     * Helper template for registering objects
     * with the Dwarf::Factory
     */
    template<typename T, Dwarf_Half tag = T::TAG>
    CLASS Producer : public T
    {
    public:
        typedef T base_type;

        enum { TAG = tag };

        static void register_with_factory(Dwarf_Half key = tag)
        {
            register_creator(key, create_instance);
        }

        virtual ~Producer() throw() {}

        Producer(Dwarf_Debug dbg, Dwarf_Die die) : T(dbg, die)
        { }

        static std::shared_ptr<Die> create_instance(Dwarf_Debug dbg, Dwarf_Die die)
        {
            std::shared_ptr<Die> ptr = std::make_shared<Producer>(dbg, die);
            return ptr;
        }
    };
}
#endif // PRODUCER_H__81BC708E_A617_4016_A149_8150BE47F425
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
