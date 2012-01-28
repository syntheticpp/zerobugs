// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <assert.h>
#include "zdk/atomic.h"


void test_simple()
{
    int foo = 0;
    assert(compare_and_swap(foo, 0, 0));
    assert(compare_and_swap(foo, 0, 1));
    assert(!compare_and_swap(foo, 0, 0));
    assert(compare_and_swap(foo, 1, 0));
    assert(compare_and_swap(foo, 0, 0));
}


int main()
{
    test_simple();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
