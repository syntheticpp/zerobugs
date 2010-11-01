// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include "zdk/string.h"



int main()
{
    assert(strcmp_ignore_space("foo", "foo") == 0);
    assert(strcmp_ignore_space("fool", "foo") == 1);
    assert(strcmp_ignore_space("foo", "fool") == -1);

    assert(strcmp_ignore_space("f oo", "foo") == 0);
    assert(strcmp_ignore_space("foo", "f oo") == 0);
    assert(strcmp_ignore_space("f o o", "f o o") == 0);
    assert(strcmp_ignore_space("foo ", "foo") == 0);
    assert(strcmp_ignore_space("foo", "foo ") == 0);

    assert(strcmp_ignore_space("fo ol", "foo") == 1);
    assert(strcmp_ignore_space("f o o", "fool") == -1);
    assert(strcmp_ignore_space("fo ol", "  foo  ") == 1);
    assert(strcmp_ignore_space(" f o o l", "  foo  ") == 1);

    assert(strcmp_ignore_space("", "") == 0);
    assert(strcmp_ignore_space(" ", "") == 0);
    assert(strcmp_ignore_space("", " ") == 0);
    assert(strcmp_ignore_space("x", "x ") == 0);
    assert(strcmp_ignore_space("x", " x") == 0);
    assert(strcmp_ignore_space("x ", " x") == 0);
    assert(strcmp_ignore_space(" x", "x ") == 0);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
