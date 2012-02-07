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

#include "zdk/shared_string_impl.h"
#include "private/generic_attr.h"
#include "private/indirect.h"
#include "const_value.h"
#include "datum.h"
#include "debug.h"
#include "utils.h"
#include "impl.h"
#include "type.h"

using namespace std;
using namespace Dwarf;


Datum::Datum(Dwarf_Debug dbg, Dwarf_Die die) : Die(dbg, die)
{
}


std::shared_ptr<Type> Datum::type() const
{
    std::shared_ptr<Type> type = Utils::type(*this);

    if (type)
    {
        assert(&owner() == &type->owner());
    }
    return type;
}


char* Datum::name_impl() const
{
    char* name = Die::name_impl();
    if (!name)
    {
        if (std::shared_ptr<Die> indirect = check_indirect())
        {
            name = strdup(indirect->name());
        }
        else if (global_)
        {
            name = strdup(global_->name());
        }
    }
    return name;
}


std::shared_ptr<Location> Datum::loc(bool indirect) const
{
    std::shared_ptr<Location> lp = Utils::loc(dbg(), die());
    if (!lp && indirect)
    {
        std::shared_ptr<Die> tmp = check_indirect(false);
        if (std::shared_ptr<Datum> dat = std::dynamic_pointer_cast<Datum>(tmp))
        {
            lp = dat->loc(indirect);
        }
    }
    return lp;
}


Dwarf_Off Datum::start_scope() const
{
    GenericAttr<DW_AT_start_scope, Dwarf_Off> attr(dbg(), die());
    return attr.is_null() ? 0 : attr.value();
}


void Datum::set_global(const std::shared_ptr<Global>& global)
{
    assert(!global_ || global_ == global);
    global_ = global;
}


RefPtr<SharedString> Datum::linkage_name() const
{
    if (linkageName_.is_null())
    {
        string name;

        if (!Utils::get_linkage_name(dbg(), die(), name))
        {
            if (std::shared_ptr<Die> tmp = check_indirect())
            {
                Utils::get_linkage_name(dbg(), tmp->die(), name);
            }
        }
        linkageName_ = shared_string(name);
    }
    return linkageName_;
}


std::shared_ptr<ConstValue> Datum::const_value() const
{
    std::shared_ptr<ConstValue> val;

    if (Utils::has_attr(dbg(), die(), DW_AT_const_value))
    {
        GenericAttr<DW_AT_const_value, Dwarf_Unsigned> attr(dbg(), die());

        if (attr.is_block())
        {
            val = std::make_shared<ConstValue>(attr.block());
        }
        else
        {
            val = std::make_shared<ConstValue>(attr.value());
        }
    }
    return val;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
