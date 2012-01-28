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
//
#include <iostream>
#include <iterator>
#include "elfz/public/dyn_lib_list.h"


using namespace std;
using namespace ELF;

int main(int argc, char* argv[])
{
    for (--argc, ++argv; argc; --argc, ++argv)
    {
        try
        {
            DynLibList libs(*argv, environ);
            copy(libs.begin(), libs.end(), ostream_iterator<string>(cout, "\n"));
        }
        catch (const exception& e)
        {
            cerr << "exception: " << e.what() << endl;
        }
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
