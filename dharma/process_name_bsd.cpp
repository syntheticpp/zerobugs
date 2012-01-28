// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <limits.h>     // MAX_PATH
#include <unistd.h>     // getpid
#include <sys/sysctl.h>
#include "process_name.h"
#include "system_error.h"

using namespace std;


void procname(pid_t pid, char name[PATH_MAX], bool throwOnError)
{
    if (pid == 0)
    {
        pid = ::getpid();
    }
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_ARGS, pid };

    size_t len = PATH_MAX - 1;

    if (sysctl(mib, 4, name, &len, NULL, 0) < 0 && throwOnError)
    {
        len = 0;
        throw SystemError("procname");
    }
    name[len] = 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
