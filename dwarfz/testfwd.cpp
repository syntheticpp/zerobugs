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

#include "test.h"
#include "public/debug.h"
#include "public/type.h"

struct Forward;

BEGIN_TEST(test_forward, (const char* self, Forward* fptr))
{
    Dwarf::Debug dbg(self);
    assert(fptr);
    boost::shared_ptr<Dwarf::Type> type = dbg.lookup_type("Forward", 0);
    assert(type);
    assert(type->is_complete());
}
END_TEST()

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
