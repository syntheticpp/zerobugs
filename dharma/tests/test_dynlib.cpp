//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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

#include <cassert>
#include <iostream>
#include "dharma/dynamic_lib.h"

using namespace std;

struct TestDynamicLib : public DynamicLib
{
    TestDynamicLib(const char* filename) : DynamicLib(filename)
    { }

    using DynamicLib::load;
};

static int myCount = 0;

extern "C" void foo()
{
    clog << __func__ << endl;
    ++myCount;
}


int main(int argc, char* argv[])
{
    try
    {
        TestDynamicLib self(argv[0]);
        ImportPtr<void ()> p;

        self.load();
        self.import("foo", p);
        assert(p.get() == &foo);

        (*p)();

        assert(myCount == 1);
        p.reset();

        assert(p.is_null());
    }
    catch (const exception& e)
    {
        clog << "Exception caught " << e.what() << endl;
        assert(false);
    }
    return 0;
}
