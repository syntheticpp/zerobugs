// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <limits.h>
#include <unistd.h>     // getpid
#include <sstream>      // ostringstream
#include "process_name.h"
#include "system_error.h"

using namespace std;


void procname(pid_t pid, char name[PATH_MAX], bool throwOnError)
{
    if (pid == 0)
    {
        pid = ::getpid();
    }
    ostringstream ostr;

    ostr  << "/proc/" << pid << "/exe";
    ssize_t n = readlink(ostr.str().c_str(), name, PATH_MAX - 1);

    if (n == -1 && throwOnError)
    {
        n = 0;
        throw SystemError(ostr.str());
    }
    name[n] = 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
