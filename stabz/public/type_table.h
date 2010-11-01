#ifndef TYPE_TABLE_H__63227AA1_AC13_40AE_A76F_14142E9738E8
#define TYPE_TABLE_H__63227AA1_AC13_40AE_A76F_14142E9738E8
//
// $Id: type_table.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <deque>
#include <boost/shared_ptr.hpp>
#include "zdk/data_type.h"

namespace Stab
{
    typedef std::deque<WeakDataTypePtr> TypeTable;
    //typedef std::deque<RefPtr<DataType> > TypeTable;

    typedef boost::shared_ptr<TypeTable> TypeTablePtr;
}

#endif // TYPE_TABLE_H__63227AA1_AC13_40AE_A76F_14142E9738E8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
