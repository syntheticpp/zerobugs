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
#include <errno.h>
#include <unistd.h>     // execvp
#include "exec.h"
#include "exec_arg.h"
#include "syscall_wrap.h"
#include "system_error.h"

using namespace std;


pid_t
exec(const string& file, const deque<string>& args, int fdOut)
{
    pid_t pid = sys::fork();

    if (pid != 0)
    {   // parent process?
        if (fdOut != 0)
        {
            close(fdOut);
        }
    }
    else
    {   // child process
        if (fdOut)
        {
            if (dup2(fdOut, STDOUT_FILENO) < 0)
            {
                _exit(errno);
            }
        }

        sys::unmask_all_signals();

        ExecArg arg(args.begin(), args.end());

        if (execvp(file.c_str(), arg.cstrings()) < 0)
        {
            _exit(errno);
        }
    }
    return pid;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
