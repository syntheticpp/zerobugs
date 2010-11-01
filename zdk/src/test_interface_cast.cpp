// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "zdk/interface_cast.h"

using namespace std;


DECLARE_ZDK_INTERFACE_(Foo, Unknown2)
{
    DECLARE_UUID("2650a62d-2642-4fbd-adfe-cf93f0944c6c")

BEGIN_INTERFACE_MAP(Foo)
    INTERFACE_ENTRY(Foo)
END_INTERFACE_MAP()
};

DECLARE_ZDK_INTERFACE_(Baz, Unknown2)
{
    DECLARE_UUID("17e5303c-5c88-48e2-8c0e-01ddc6d81ab0")
};

class Bar : public Unknown2
{
    Foo foo_;

BEGIN_INTERFACE_MAP(Bar)
    INTERFACE_ENTRY_DELEGATE(&foo_)
END_INTERFACE_MAP()

public:
    Bar() { }
};


int main()
{
    Bar bar;

    assert(static_cast<Unknown2*>(&bar));
    assert(interface_cast<Foo*>(&bar));
    assert(interface_cast<Baz*>(&bar) == 0);
    assert(interface_cast<Baz*>((Foo*)0) == 0);

    try
    {
        interface_cast<Foo&>(bar);
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        return -1;
    }
    assert(true);
    try
    {
        interface_cast<Baz&>(bar);
        assert(false);
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
    }
    return 0;
}
