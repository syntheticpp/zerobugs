
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "test_common.h"
#include <iostream>
#include "generic/auto_handle.h"

class A
{
public:
    A() { std::clog << "A\n"; }
    ~A() { std::clog << "~A\n"; }
};

template<>
struct handle_traits<A*>
{
    static A* null_value() { return 0; }
    static void dispose(A* ptr) { delete ptr; }
};

template<class H, class T>
class exclusive_base_handle
{
public:
    explicit exclusive_base_handle(H) {}

protected:
    static void dispose(H h) throw()
    { T::dispose(h); }

    static H copy(H& h) throw()
    {
        H result = h;
        h = T::null_value();

        return result;
    }

    // Place-holder. This is needed in the case
    // where a class that maintains some state
    // is used instead of exclusive_base_handle.
    static void swap(exclusive_base_handle&) throw() {}

    static H release(H& h) throw()
    {
        H res(h);
        h = T::null_value();
        return res;
    }
};

typedef auto_handle<A*, handle_traits<A*>,
    exclusive_base_handle<A*, handle_traits<A*> > > Handle;

int main()
{
    Handle h1(new A);
    {
        Handle h2 = h1;
        assert(h1.get() == 0);
        assert(h2.get() != 0);
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
