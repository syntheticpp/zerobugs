//
// $Id: abi_x86.cpp 713 2010-10-16 07:10:27Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "abi.h"

using namespace std;


void Dwarf::ABI::out_of_range(const char* fun, size_t n)
{
#if 0
    ostringstream err;

    err << fun << ": register index " << n << " is out of range";

    throw std::out_of_range(err.str());
#else
    cerr << "***** register index " << n << " is out of range *****\n";
#endif
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
