//
// $Id: is_cv_qualified.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "public/types.h"
#include "public/is_cv_qualified.h"


bool is_cv_qualified(const DataType* type)
{
    return interface_cast<const ConstType*>(type)
        || interface_cast<const VolatileType*>(type);
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
