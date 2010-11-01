// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>     // getpid
#include <sstream>      // ostringstream
#include <vector>
#include "process_name.h"

using namespace std;

extern void procname(pid_t pid, char name[PATH_MAX], bool throwOnError);


string get_process_name(pid_t pid)
{
    char name[PATH_MAX] = { 0 };

    procname(pid, name, true /* throw exceptions */);
    return name;
}


string get_process_name(nothrow_t, pid_t pid)
{
    char name[PATH_MAX] = { 0 };

    procname(pid, name, false);

    return name;
}


string realpath_process_name(pid_t pid)
{
    vector<char> buf(4096); // a raw buffer

    string name(get_process_name(pid));

    // if realpath fails, the contents of buf are undefined
    if (!::realpath(name.c_str(), &buf[0]))
    {
        return name;
    }
    assert(buf[0] == '/');
    return string(&buf[0]);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
