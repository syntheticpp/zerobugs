// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include <stdexcept>
#include <stdarg.h>
#include <math.h>

using namespace std;

int do_something(bool);

int bar(int x, int y, ...)
{
    va_list v;
    va_start(v, y);
    va_end(v);
    if (getenv("ABORT"))
        abort();
    do_something(getenv("CRASH"));
    return x + y;
}

int fun(long a, int b, short c)
{
    std::cout << "a=" << a << endl;
    std::cout << "b=" << b << endl;
    std::cout << "c=" << c << endl;
    if (a > 100)
    {
        long d = a + b + c;
        return d / 2;
    }
    return bar(a, b);
}

void fuzzy()
{
    cout << __func__ << endl;
}

long double baz(const int& x)
{
    cout << __func__ << endl;
    return x + fun(bar(1,x), 2, 3);
}


void do_throw()
{
    throw runtime_error("bloody blah");
}

int do_something(bool f)
{
    if (f)
    {
        *(int*)0 = 1;
    }
    return 123;
}

int main(int argc, char* argv[])
{
#ifdef __GLIBC__
    cout << "__GLIBC__" << __GLIBC__ << endl;
#endif
    for (--argc, ++argv; argc; --argc, ++argv)
    {
        cout << *argv << endl;
    }
    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int b = 1000;
    int esp = 123;

    double d = 4.2;

    esp = baz(labs(static_cast<const int&>(d)));
    d += .2 + b + esp;

    cout << d << endl;
    cout << (int*)a << endl;
    for (int i = 0; b > 100; ++i)
    {
        b = fun(abs(b), b / 2, a[i]);
    }
    return baz(40) + bar(1, 2/*, 3, 4*/);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
