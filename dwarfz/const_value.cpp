// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------//
//
// $Id$
//
#include <string.h>
#include "const_value.h"


Dwarf::ConstValue::ConstValue(const Buffer& data) : data_(data)
{
}



Dwarf::ConstValue::ConstValue(Dwarf_Unsigned val)
{
    data_.resize(sizeof val);
    memcpy(&data_[0], &val, sizeof val);
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
