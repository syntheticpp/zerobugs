//
// $Id: variable.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "variable.h"


////////////////////////////////////////////////////////////////
Dwarf::Variable::Variable(Dwarf_Debug dbg, Dwarf_Die die)
    : Datum(dbg, die)
{
}


Dwarf::Variable::~Variable() throw()
{
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
