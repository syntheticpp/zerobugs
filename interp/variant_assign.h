#ifndef VARIANT_ASSIGN_H__DB04AF40_003C_4EC1_B242_4D256C9DB42B
#define VARIANT_ASSIGN_H__DB04AF40_003C_4EC1_B242_4D256C9DB42B
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// ------------------------------------------------------------------------

#include <sstream>
#include "zdk/data_type.h"
#include "zdk/variant.h"

template<typename T>
inline void
variant_assign(Unknown2& unk, const DataType& type, const T& value)
{
    std::ostringstream os;
    os << value;

    type.parse(os.str().c_str(), &unk);
}
#endif // VARIANT_ASSIGN_H__DB04AF40_003C_4EC1_B242_4D256C9DB42B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
