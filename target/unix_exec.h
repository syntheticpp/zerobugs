#ifndef UNIX_EXEC_H__4BF4E70E_58FF_11DA_B82B_00C04F09BBCC
#define UNIX_EXEC_H__4BF4E70E_58FF_11DA_B82B_00C04F09BBCC
//
// $Id: unix_exec.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// ------------------------------------------------------------------------

#include <cassert>
#include <signal.h>
#include "unix.h"
#include "zdk/check_ptr.h"
#include "dharma/exec_arg.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"

namespace Unix
{
    inline void unmask_all_signals()
    {
        sigset_t mask;
        sigemptyset(&mask);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }

    template<typename T>
    static RefPtr<Thread> attach(T& target)
    {
        int status = 0;

        Process* process = CHKPTR(target.process());

        const pid_t event_pid = sys::waitpid(process->pid(), &status, 0);
        assert(event_pid == process->pid()); // by sys::waitpid contract

        // check that the process is well and alive
        if (WIFEXITED(status) /* || WIFSIGNALED(status) */)
        {
            // fixme: what if exit status is zero?
            throw SystemError(WEXITSTATUS(status));
        }

        // reading the symbols is purposely deferred
        // until after waitpid so that if waitpid fails
        // we don't waste any time doing it
        target.init_symbols();

        pid_t lwpid = target.pid_to_lwpid(event_pid);
        RefPtr<Thread> thread = target.new_thread(0, lwpid, status);
        // target.update_symbol_map(*thread);
        return thread;
    }


    /**
     * Encapsulates the mechanics for executing a process
     * under UNIX, and attaching to it.
     */
    template<typename T>
    static RefPtr<Thread>
    exec(T& target, const ExecArg& arg, const char* const* env)
    {
        RefPtr<Thread> thread;

        target.detach();
        assert(target.enum_threads() == 0);

        if (pid_t pid = sys::fork())
        {
            target.init_process(pid, &arg, ORIGIN_DEBUGGER);

            // NOTE attaching to a new process is slightly different
            // from attaching to a process that is already running;
            // here, we attach to the new process.
            thread = Unix::attach(target);
        }
        else // child process
        {
            target.close_all_files();

            unmask_all_signals();
            XTrace::exec(arg.cstrings(), env);
        }
        return thread;
    }
} // namespace unix

#endif // UNIX_EXEC_H__4BF4E70E_58FF_11DA_B82B_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
