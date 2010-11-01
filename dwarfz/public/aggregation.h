#ifndef AGGREGATION_H__B614DC88_EC26_4C6C_A981_7FF896D8D1A5
#define AGGREGATION_H__B614DC88_EC26_4C6C_A981_7FF896D8D1A5
//
// $Id: aggregation.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "interface.h"
#include "access.h"
#include "die.h"

namespace Dwarf
{
    class Location;
    class Type;

    /**
     * Base class for the
     * DW_TAG_member and DW_TAG_inheritance wrappers.
     */
    CLASS  Aggregation : public Die
    {
    public:
        virtual ~Aggregation() throw();

        Access access() const;

        boost::shared_ptr<Location> loc() const;

        boost::shared_ptr<Type> type() const;

    protected:
        Aggregation(Dwarf_Debug, Dwarf_Die);

    private:
        mutable boost::shared_ptr<Type> type_;
    };
}
#endif // AGGREGATION_H__B614DC88_EC26_4C6C_A981_7FF896D8D1A5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
