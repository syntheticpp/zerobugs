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
#include "../process_name.h"
#include "../canonical_path.h"

using namespace std;

int main(int argc, char* argv[])
{
    cout << get_process_name(getpid()) << endl;
    cout << canonical_path(argv[0]) << endl;
    assert(get_process_name(getpid()) == canonical_path(argv[0]));
}
