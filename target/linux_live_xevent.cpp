//
// $Id: linux_live_xevent.cpp 720 2010-10-28 06:37:54Z root $
//
// Handle extended ptrace events.
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Handle ptrace extension events.
//
#include <errno.h>
#include <sys/wait.h>
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "dharma/environ.h"
#include "dharma/process_name.h"
#include "dharma/syscall_wrap.h"
#include "debugger_base.h"
#include "linux_live.h"
#include "ptrace.h"
#include "thread.h"
#include "unix_exec.h"

using namespace std;
using namespace eventlog;


/**
 * @return true if the debugger may continue without stopping
 * on this event
 */
bool
LinuxLiveTarget::handle_extended_event(Thread& thread, int event)
{
    word_t pid = 0;
    if (event == PTRACE_EVENT_EXEC)
    {
        pid = thread.lwpid();
    }
    else
    {
        sys::ptrace(PT_GETEVENTMSG, thread.lwpid(), 0, (word_t)&pid);
    }
    dbgout(0) << __func__ << ": " << pid << " event=" << event << endl;

    switch (event)
    {
    case PTRACE_EVENT_CLONE:
        {
            const size_t nThreads = enum_threads();
            bool createThread = false;

            dbgout(0) << "PTRACE_EVENT_CLONE, currently got "
                      << nThreads << " thread(s)" << endl;

            if (nThreads > 1)
            {
                createThread = true;
            }
            else
            {
                // Make sure that event-reporting is activated;
                // for dynamically linked target this also results
                // in calling on_thread() which creates a new thread.
                iterate_threads(*this);

                if (enum_threads() <= 1)
                {
                    createThread = true;
                }
            }

            if (createThread)
            {
                const int status = 0;
                /////
                //sys::waitpid(pid, &status, __WCLONE);
                ///// No need to waitpid, passing a zero status
                ///// to create_thread triggers wait_update_status

                create_thread(0, pid, status);
            }
        }
        break;

    case PTRACE_EVENT_FORK:
        assert(!debugger().get_thread(pid)); //must not be already attached
        {
            const int wordSize = thread.is_32_bit() ? 32 : __WORDSIZE;

            // make a new target for the forked process
            RefPtr<LinuxLiveTarget> target = new LinuxLiveTarget(debugger(), wordSize);

            debugger().add_target(target);

            Temporary<bool> setFlag(target->resumeNewThreads_, resumeNewThreads_);
            RefPtr<SharedString> cmd = CHKPTR(process())->command_line();

            // create the forked process with same arguments as parent
            ExecArg args(cmd ? cmd->c_str() : process()->name());
            target->init_process(pid, &args, process()->origin(), process()->name());
            target->init_symbols(symbols());

            RefPtr<Thread> thread = target->handle_fork(pid);
            assert(target->get_thread(pid, 0) == thread.get());

            if (debugger().options() & Debugger::OPT_SPAWN_ON_FORK)
            {
                debugger().cleanup(*thread);
            #if 0
                sys::ptrace(PTRACE_DETACH, thread->lwpid(), 0, SIGSTOP);
            #else
                XTrace::kill(thread->lwpid(), SIGSTOP);
                sys::ptrace(PTRACE_DETACH, thread->lwpid(), 0, 0);
            #endif
                ostringstream cmd;
                cmd << env::get("ZERO_EXE", "zero") << " --fork --spawn-on-fork " << pid;
                cmd << " >> zero_spawn.log 2>&1 ";
                cmd << "&"; // start in background
                dbgout(0) << cmd.str() << endl;

                Unix::unmask_all_signals();

                if (system(cmd.str().c_str()) < 0)
                {
                    dbgout(0) << "system call failed, errno=" << errno << endl;
                    throw SystemError(cmd.str());
                }
            }
        }
        break;

    case PTRACE_EVENT_EXEC:
        {
            assert(thread.target() == this);
            assert(thread.is_forked());

            RefPtr<Target> self(this);

            cleanup(thread);

            assert(get_thread(pid) == 0);
            handle_exec(pid);

            assert(get_thread(pid));
        }
        break;

    case PTRACE_EVENT_EXIT:
      #ifdef DEBUG
        clog << "*** Thread exiting: " << thread.lwpid() << " ***\n";
      #endif
        //do not cleanup the thread here, let the exit event
        //come in through waitpid()
        interface_cast<ThreadImpl&>(thread).set_exiting();
        break;
    }
    return true;
}


////////////////////////////////////////////////////////////////
bool LinuxLiveTarget::check_extended_event(Thread* thread)
{
    bool result = false;

    if (thread)
    {
        if (const int event = (thread->status() >> 16))
        {
            assert(((thread->status() >> 8) & 0xff) == SIGTRAP);

            if (event != PTRACE_EVENT_EXEC)
            {
                Temporary<bool> setFlag(resumeNewThreads_, true);
                assert(resumeNewThreads_ == true);

                result = handle_extended_event(*thread, event);
            }
        }
    }
    return result;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
