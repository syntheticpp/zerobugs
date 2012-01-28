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
#include "public/variable.h"
#if DEBUG
 #include <iostream>
 using namespace std;
#endif

Stab::Variable::Variable
(
    SharedString&   name,
    DataType&       type,
    addr_t          offset
)
: Datum(name, type)
, offset_(offset)
{
}


addr_t Stab::Variable::addr(addr_t, addr_t frameBase) const
{
    return frameBase + offset();
}


Stab::Parameter::Parameter
(
    SharedString&   name,
    DataType&       type,
    addr_t          offset
)
: Variable(name, type, offset)
{
}


Stab::GlobalVariable::GlobalVariable
(
    SharedString&   name,
    DataType&       type,
    addr_t          offset
)
: Variable(name, type, offset)
{
}


addr_t Stab::GlobalVariable::addr(addr_t vmBase, addr_t) const
{
    return offset() + vmBase;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
