//
// $Id$
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
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include <errno.h>
#include <sys/wait.h>
#include "generic/temporary.h"
#include "dharma/environ.h"
#include "dharma/process_name.h"
#include "dharma/syscall_wrap.h"
#include "debugger_base.h"
#include "linux_live.h"
#include "ptrace.h"
#include "thread.h"
#include "unix_exec.h"

using namespace std;


static void attach_new_debugger_instance(pid_t pid)
{
    // construct command line
    ostringstream cmd;
    cmd << env::get("ZERO_EXE", "zero") << " --fork --spawn-on-fork --no-banner ";
    cmd << pid;
    cmd << " >> zero_fork.out 2>&1 ";
    cmd << "&"; // start in background
#if DEBUG
    clog <<__func__ << ": " << cmd.str() << endl;
#endif
    sys::unmask_all_signals();

    // execute
    if (system(cmd.str().c_str()) < 0)
    {
        throw SystemError(cmd.str());
    }
}

void LinuxLiveTarget::on_clone(pid_t pid)
{
    const size_t nThreads = enum_threads();
    bool createThread = false;

#if DEBUG
    clog << __func__ << ": got " << nThreads << " thread(s)" << endl;
#endif
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

        // passing a status=0 to create_thread causes
        // a wait_update_status call to update status
        create_thread(0, pid, status);
    }
}


void LinuxLiveTarget::on_fork(pid_t pid, size_t wordSize, int status)
{
#if DEBUG
    clog << "fork " << wordSize << "-bit target" << endl;
#endif
    // make a new target for the forked process
    RefPtr<LinuxLiveTarget> target = new LinuxLiveTarget(debugger(), wordSize);

    debugger().add_target(target);

    Temporary<bool> setFlag(target->resumeNewThreads_, resumeNewThreads_);
    RefPtr<SharedString> cmd = CHKPTR(process())->command_line();

    // create the forked process with same arguments as parent
    ExecArg args(cmd ? cmd->c_str() : process()->name());
    target->init_process(pid, &args, process()->origin(), process()->name());
    target->init_symbols(symbols());

    RefPtr<Thread> thread = target->handle_fork(pid, status);
    assert(target->get_thread(pid, 0) == thread.get());

    // NOTE: This should probably be moved to the main engine,
    // or to a base class so we don't have to copy and paste
    // the code when we implement support for other target OS-es.

    if (debugger().options() & Debugger::OPT_SPAWN_ON_FORK)
    {
        debugger().cleanup(*thread);
        sys::ptrace(PTRACE_DETACH, thread->lwpid(), 0, SIGSTOP);

        attach_new_debugger_instance(pid);
    }
}


/**
 * @return true if the debugger may continue without stopping
 * on this event
 */
bool LinuxLiveTarget::handle_extended_event(Thread& thread, int event)
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
        on_clone(pid);
        break;

    case PTRACE_EVENT_FORK:
        // must not be already in our list of threads
        assert(!debugger().get_thread(pid)); 
        on_fork(pid, thread.is_32_bit() ? 32 : __WORDSIZE, 0);

        break;

    case PTRACE_EVENT_EXEC:
        assert(thread.target() == this);

        {
            RefPtr<Target> self(this);

            cleanup(thread);

            assert(get_thread(pid) == 0);

            handle_exec(pid);

            assert(get_thread(pid));
        }
        break;

    case PTRACE_EVENT_EXIT:
        dbgout(0) << "Thread exiting: " << thread.lwpid() << endl;

        //do not cleanup the thread here, let the exit event
        //come in through waitpid()
        interface_cast<ThreadImpl&>(thread).set_exiting();
        break;

    case PTRACE_EVENT_VFORK:
        throw runtime_error("PTRACE_EVENT_VFORK is not supported");
    
    case PTRACE_EVENT_VFORK_DONE:
        throw runtime_error("PTRACE_EVENT_VFORK_DONE is not supported");

    default:
        clog << __func__ << ": unhandled event=" << event << endl;
        assert(false);
        return false;
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
