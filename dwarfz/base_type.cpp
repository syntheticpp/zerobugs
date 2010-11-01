//
// $Id: base_type.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <dwarf.h>
#include <boost/limits.hpp>
#include "error.h"
#include "factory.h"
#include "private/encoding_attr.h"

#include "impl.h"
#include "base_type.h"

using namespace Dwarf;


BaseType::BaseType(Dwarf_Debug dbg, Dwarf_Die die) : Type(dbg, die)
{
}


BaseType::Encoding BaseType::encoding() const
{
    EncodingAttr attr(dbg(), die());
    Dwarf_Unsigned enc = attr.value();

    if (enc > DW_ATE_hi_user)
    {
        enc = DW_ATE_hi_user;
    }
    return static_cast<BaseType::Encoding>(enc);
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
