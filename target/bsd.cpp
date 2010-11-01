// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: bsd.cpp 714 2010-10-17 10:03:52Z root $
//
#include "bsd.h"


BSDTarget::BSDTarget(debugger_type& dbg) : UnixTarget(dbg)
{
}


void BSDTarget::close_all_files()
{
    // todo
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
