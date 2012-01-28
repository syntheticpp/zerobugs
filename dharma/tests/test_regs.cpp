// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "test_common.h"
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include "zdk/config.h"
#if HAVE_ASM_UNISTD_H
 #include <asm/unistd.h>
#endif
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"

using namespace std;


int main(int argc, char* argv[])
{
    try
    {
        const pid_t pid = fork();
        if (pid == 0)
        {
            sleep(INT_MAX);
        }
        else
        {
            cout << "pid=" << pid << endl;

    #if defined(__MACH__)
            mach_port_t task = 0;
            int err = task_for_pid(mach_task_self(), pid, &task);
            if (err == KERN_SUCCESS)
            {
                cout << "task=" << task << endl;
                err = task_suspend(task);
                cout << "task_suspend() err=" << err << endl;
                thread_act_port_array_t threads = 0;
                mach_msg_type_number_t count = 0;
                err = task_threads(task, &threads, &count);
                cout << "err=" << err << " count=" << count << endl;
                if (err == 0)
                {
                    cout << "thread[0]=" << threads[0] << endl;
                }
                user_regs_struct uregs;
                sys::get_regs(threads[0], uregs);
                return 0;
            }
            else
            {
                cout << "mask_task_for_pid() err=" << err << endl;
            }

    #endif // __MACH__

            sys::ptrace(PTRACE_ATTACH, pid, 0, 0);
            cout << "PTRACE_ATTACH ok" << endl;

            sys::waitpid(pid, 0, 0);
            user_regs_struct uregs;

            cout << "fetching CPU registers..." << endl;
            sys::get_regs(pid, uregs);

    #if defined(__linux__)
        #if defined  (__i386__)
            cout << "orig_eax=" << uregs.orig_eax << endl;
            assert(uregs.orig_eax == __NR_nanosleep || uregs.orig_eax == __NR_fork);
        #elif defined(__PPC__)
            cout << "orig_gpr3=" << uregs.orig_gpr3 << endl;
            cout << "gpr3=" << uregs.gpr[3] << endl;
        #endif
            user_fpxregs_struct fpxregs;
            sys::get_regs(pid, fpxregs);
            sys::ptrace(PTRACE_KILL, pid, 0, 0);
            sys::waitpid(pid, 0, 0);
    #endif // __linux
        }
    }
    catch (SystemError& e)
    {
        cerr << e.what() << endl;
        assert(false);
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        assert(false);
    }
    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
