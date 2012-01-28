// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iostream>
#include <unistd.h>
#include "../directory.h"
#include "../canonical_path.h"
#include "../syscall_wrap.h"


using namespace std;

int main(int argc, char* argv[])
{
    const Directory procDir(".", "*.cpp");
    assert(procDir.size() == 9);

    for (auto i = procDir.begin(); i != procDir.end(); ++i)
    {
        cout << *i << endl;
    }
    return 0;
}
