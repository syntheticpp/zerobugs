#ifndef TEMPLATE_TYPE_H__2CEE9BE6_46F8_4B72_A43E_B4951665A78E
#define TEMPLATE_TYPE_H__2CEE9BE6_46F8_4B72_A43E_B4951665A78E
//
//
// $Id: template_type.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "child.h"
#include "die.h"
#include "iterator.h"
#include "utils.h"


namespace Dwarf
{
    template<typename T>
    CLASS TemplateType : public Child<T>, public Die
    {
    public:

        enum { TAG = DW_TAG_template_type_parameter };
        friend class IterationTraits<TemplateType<T> >;

        boost::shared_ptr<Type> type() const
        {
            return Utils::type(this->dbg(), this->die());
        }

    protected:
        TemplateType(Dwarf_Debug dbg, Dwarf_Die die)
            : Die(dbg, die)
        { }

    };
}

#endif // TEMPLATE_TYPE_H__2CEE9BE6_46F8_4B72_A43E_B4951665A78E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
