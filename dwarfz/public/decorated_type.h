#ifndef DECORATED_TYPE_H__21F79B87_9F9F_4E2E_82A4_4DC0B4CB175C
#define DECORATED_TYPE_H__21F79B87_9F9F_4E2E_82A4_4DC0B4CB175C
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

#include "interface.h"
#include "type.h"

namespace Dwarf
{
/**
 * A base class for "decorated" types -- see the "Decorator Pattern";
 * a decorated type may add a const or volatile (or both) qualifier
 * to an existent type, or define a pointer or reference type.
 * @note Unlike others in the Type-based hierarchy, this class does
 * not model a C/C++ type; it rather serves as an intermediary base
 * for ConstType, VolatileType and PointerType.
 */
    CLASS DecoratedType : public Type
    {
    public:
        virtual ~DecoratedType() throw() {}

        /**
         * @return the decorated type; attempts to deal with
         * incomplete types.
         */
        virtual std::shared_ptr<Type> type() const;

        virtual bool is_pointer_or_ref() const;

    protected:
        DecoratedType(Dwarf_Debug, Dwarf_Die);

    private:
        friend class Utils;

        mutable std::shared_ptr<Type> type_;
    };
}

#endif // DECORATED_TYPE_H__21F79B87_9F9F_4E2E_82A4_4DC0B4CB175C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
