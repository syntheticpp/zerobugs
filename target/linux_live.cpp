//
// $Id: linux_live.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <fstream>
#include <sstream>
#include "generic/auto_file.h"
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/signal_policy.h"
#include "zdk/thread_util.h"
#include "zdk/variant_util.h"
#include "zdk/32_on_64.h"
#include "dharma/canonical_path.h"
#include "dharma/directory.h"
#include "dharma/environ.h"
#include "dharma/process_name.h"
#include "dharma/sigutil.h"
#include "dharma/syscall_wrap.h"
#include "dharma/virtual_dso.h"
#include "elfz/public/binary.h"
#include "elfz/public/headers.h"
#include "symbolz/public/symbol_map.h"
#include "debugger_base.h"
#include "target/target_factory.h"
#include "target/linux_live.h"
#include "target/memory_access.h"
#include "target/reg.h"
#include "target/unix_exec.h"
#include "target/utility.h"
#include "process.h"
#include "ptrace.h"
#include "thread.h"

using namespace std;
using namespace eventlog;


// Use the signfo_t struct to determine for sure
// that the signal came from us.
static pid_t get_signal_sender_pid(pid_t lwpid)
{
    siginfo_t siginfo = { 0 };
    if (XTrace::ptrace(PT_GETSIGINFO, lwpid, 0, (addr_t)&siginfo) == 0)
    {
        return siginfo.si_pid;
    }
    return -1;
}


static bool set_stopped(RefPtr<ThreadImpl> thread, bool expectStop = false)
{
    if (thread->signal() == SIGSTOP)
    {
        assert( !thread->is_stop_expected() );

        const pid_t pid = thread->get_signal_sender();

        if (pid == 0 || pid == getpid() || expectStop)
        {
            thread->set_stopped_by_debugger(true);
            return true;
        }
    }
    else if (expectStop)
    {
        thread->set_stop_expected(true);
    }
    return false;
}


static RefPtr<Target> create(debugger_type& debugger)
{
    return new LinuxLiveTarget(debugger, __WORDSIZE);
}


/**
 * register native Linux target with factory
 */
void init_live_target()
{
    TheTargetFactory::instance().register_target(
        TargetFactory::Linux,
        __WORDSIZE,
        true,
        string(),
        create);
}



////////////////////////////////////////////////////////////////
LinuxLiveTarget::LinuxLiveTarget(debugger_type& debugger, unsigned  ws)
    : Super(debugger)
    , resumeNewThreads_(false)
    , readingAuxVect_(false)
    , kind_(K_UNKNOWN)
    , mainThreadUnique_(0)
    , oldKernel_(-1)
{
    set_word_size(ws);
}


////////////////////////////////////////////////////////////////
LinuxLiveTarget::~LinuxLiveTarget() throw()
{
}


////////////////////////////////////////////////////////////////
void LinuxLiveTarget::read_memory (
    pid_t       pid,
    SegmentType seg,
    addr_t      addr,
    long*       buf,
    size_t      len,
    size_t*     nRead ) const
{
    Memory<>::read(pid, seg, addr, buf, len, nRead, procfs_root());
}


////////////////////////////////////////////////////////////////
void LinuxLiveTarget::write_memory (
    pid_t       pid,
    SegmentType seg,
    addr_t      addr,
    const long* buf,
    size_t      len )
{
    Memory<>::write(pid, seg, addr, buf, len);
}


////////////////////////////////////////////////////////////////
Thread* LinuxLiveTarget::exec(
    const ExecArg&      arg,
    const char* const*  env)
{
    RefPtr<Thread> thread = Unix::exec(*this, arg, env);

    assert(thread);
    assert(thread->ref_count() > 1);

    if (this->kind() == K_NATIVE_32BIT)
    {
        set_word_size(32);
    }
    else
    {
        assert(word_size() == __WORDSIZE);
    }
    return thread.get();
}


////////////////////////////////////////////////////////////////
RefPtr<ProcessImpl> LinuxLiveTarget::new_process(
    pid_t           pid,
    const ExecArg*  arg,
    ProcessOrigin   orig)
{
    const string* cmdline = NULL;
    string pname; // debugged process name

    if (arg)
    {
        cmdline = &arg->command_line();
        // for a short while, the link in /proc/<pid>/exe may point
        // to the parent's; so if we have command line arguments,
        // it's a safer source for deducing the name of the exe
        pname = canonical_path((*arg)[0]);
    }
    else
    {
        pname = get_process_name(pid);
    }
    clog << __func__ << ": " << pname << endl;

    return new ProcessImpl(*this, pid, orig, cmdline, pname.c_str());
}


////////////////////////////////////////////////////////////////
RefPtr<Thread> LinuxLiveTarget::new_thread(
    long    id,
    pid_t   lwpid,
    int     status)
{
    return create_thread(id, lwpid, status);
}


////////////////////////////////////////////////////////////////
RefPtr<ThreadImpl> LinuxLiveTarget::handle_fork(

    pid_t   lwpid,
    int     status )

{
    RefPtr<ThreadImpl> thread(new ThreadImpl(*this, 0, lwpid));
    if (status)
    {
        thread->set_status(status);
    }
    else
    {
        thread->wait_update_status(true);
    }
    thread->set_forked();

    set_stopped(thread);
    set_ptrace_options(lwpid);

    add_thread(thread);
    init_thread_agent();

    bool failed = false;

    // NOTE: order of operations is important: on_attach()
    // creates the breakpoint manager for the forked thread;
    // init_linker_events() sets a breakpoint and it expects
    // that the correct manager object has been created.
   
    try
    {
        debugger().on_attach(*thread);
        init_linker_events(*thread);
    }
    catch (const exception& e)
    {
        failed = true;
        debugger().message(e.what(), Debugger::MSG_ERROR, thread.get());
    }

    if (failed)
    {
        thread->set_signal(SIGKILL);
    }

    if (resumeNewThreads_
        && (debugger().options() & Debugger::OPT_SPAWN_ON_FORK) == 0)
    {
        dbgout(0) << "resuming new thread (" << thread->lwpid() << ")" << endl;
        thread->resume();
    }
    return thread;
}


////////////////////////////////////////////////////////////////
RefPtr<ThreadImpl> LinuxLiveTarget::handle_exec(pid_t pid)
{
    if (BreakPointManager* mgr = debugger().breakpoint_manager())
    {
        if (Process* p = process())
        {
            mgr->reset(p->pid());
        }
    }

    reset_process_name();

    init_process(pid, NULL, ORIGIN_DEBUGGER);

    assert(process()->pid() == pid);
    clog << __func__ << ": " << pid << endl;

    init_symbols();

    RefPtr<ThreadImpl> thread(new ThreadImpl(*this, 0, pid));

    thread->refresh();
    thread->set_status(__W_STOPCODE(0));
    assert(thread->status() == 0x7F);

    thread->set_forked();
    thread->set_execed();

    set_ptrace_options(pid);
    uninitialize_linker_events();

    add_thread(thread);
    debugger().on_attach(*thread);

    init_thread_agent(true);

    string msg = "exec(";
           msg += thread->filename();
           msg += ")";

    clog << __func__ << ": " << process()->name() << endl;

    // stop in debugger, to give the user a chance to
    // set breakpoints
    thread_set_event_description(*thread, msg.c_str());
    debugger().queue_event(thread);
    return thread;
}


////////////////////////////////////////////////////////////////
RefPtr<ThreadImpl> LinuxLiveTarget::create_thread(
    long    id,
    pid_t   lwpid,
    int     status)
{
    RefPtr<ThreadImpl> thread(new ThreadImpl(*this, id, lwpid));

    if (status == 0)
    {
        thread->wait_update_status(true);
    }
    else
    {
        thread->set_status(status);
    }
    set_stopped(thread);
    set_ptrace_options(lwpid);
    add_thread(thread);
    debugger().add_target(this);
    debugger().on_attach(*thread);

    if (resumeNewThreads_)
    {
        dbgout(0) << __func__ << ": resuming new thread" << endl;

        thread->resume();
    }
    return thread;
}


////////////////////////////////////////////////////////////////
/// extract major and minor
static void get_release(const char* release, long& major, long& minor)
{
    char* p = 0;

    major = strtol(release, &p, 0);
    assert(p && *p == '.');
    minor = strtol(++p, 0, 0);
}


////////////////////////////////////////////////////////////////
string LinuxLiveTarget::get_system_release() const
{
    utsname sysinfo;

    if (uname(&sysinfo) < 0)
    {
        throw SystemError(__func__);
    }
    return sysinfo.release;
}


////////////////////////////////////////////////////////////////
void LinuxLiveTarget::set_ptrace_options(pid_t pid)
{
    if (oldKernel_ < 0)
    {
        long major = 0, minor = 0;
        get_release(get_system_release().c_str(), major, minor);

        oldKernel_ = (major < 2) || (major == 2 && minor < 6);

        dbgout(0) << __func__ << ": " << major << " " << minor
                  << " old_kernel=" << oldKernel_ << endl;
    }

    int options = PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT | PTRACE_O_TRACESYSGOOD;

    if (debugger().trace_fork())
    {
        options |= PTRACE_O_TRACEFORK;
        //options |= PTRACE_O_TRACEVFORK;
        //options |= PTRACE_O_TRACEVFORKDONE;
        options |= PTRACE_O_TRACEEXEC;
    }

    if (options)
    {
        const __ptrace_request req = oldKernel_ ? PT_OLDSETOPTIONS : PT_SETOPTIONS;

        while (XTrace::ptrace(req, pid, 0, options) < 0)
        {
            if (errno != EINTR)
            {
            #ifdef DEBUG
                clog << __func__ << " failed, errno=" << errno << endl;
            #endif
            #if 0
                return;
            #else
                throw SystemError(__func__, errno);
            #endif
            }
        }
        dbgout(0) << __func__ << ": " << hex << options << dec << endl;
    }
}


////////////////////////////////////////////////////////////////
static const char yama_msg[] = 
    "Ubuntu Maverick Meerkat (10.10) introduced a patch to disallow\n"
    "ptracing of non-child processes by non-root users. Only a\n"
    "process which is a parent of another process can ptrace it for\n"
    "normal users; root can still ptrace every process.\n\n"
    "You can disable this restriction by executing:\n\n"
    "echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope\n";

static void yama_check(const SystemError& e)
{
    if (e.error() == EPERM)
    {
        auto_fd f(open("/proc/sys/kernel/yama/ptrace_scope", O_RDONLY));
        if (f.is_valid())
        {
            char buf[8] = { 0 };

            try
            {
                sys::read(f.get(), &buf, 1);
            }
            catch (const exception& x)
            {
                clog << __func__ << ": " << x.what() << endl;
            }

            if (buf[0] == '1')
            {
                string s(e.what());

                s += "\n\n";
                s += yama_msg;

                throw runtime_error(s.c_str());
            }
        }
    #if DEBUG
        else
        {
            clog << "failed to open yama file" << endl;
        }
    #endif
    }

    throw e;
}

/**
 * Check whether the process is stopped before attaching;
 * resume if necessary (by sending a SIGCONT signal)
 * @return true if SIGCONT was sent
 */
static bool attach(pid_t pid, Target& target)
{
    RunnableImpl task(pid, &target);

    if (task.runstate() == Runnable::ZOMBIE)
    {
        throw runtime_error("cannot attach to defunct process");
    }

    try
    {
        sys::ptrace(PTRACE_ATTACH, pid, 0, 0);
    }
    catch (const SystemError& e)
    {
        yama_check(e);
    }

    bool sigcont = false;

    // thread was possibly in a stopped state before attaching
    if (task.runstate() == Runnable::TRACED_OR_STOPPED)
    {
        task.resume();
        sigcont = true;
    }
    return sigcont;
}

////////////////////////////////////////////////////////////////
void LinuxLiveTarget::attach(pid_t pid)
{
    assert(enum_threads() == 0); // pre-condition

    const bool sigcont = ::attach(pid, *this);
    dbgout(0) << __func__ << "(" << pid << ") sigcont=" << sigcont << endl;

    int status = 0;
    sys::waitpid(pid, &status, __WALL);
    
    assert((status >> 16) == 0);

    init_process(pid, NULL, ORIGIN_SYSTEM);
    assert(process());

    init_symbols();
    assert(symbols());

    RefPtr<ThreadImpl> thread(create_thread(0, pid, status));

    if (this->kind() == K_NATIVE_32BIT)
    {
        set_word_size(32);
    }

    if (debugger().initial_thread_fork())
    {
    #if DEBUG
        clog << "************ initial thread fork *************\n";
    #endif
        // do not clear the flag yet -- do it later in the 
        // debugger_base.cpp event loop
        // debugger().set_initial_thread_fork(false);

        thread->set_forked();
        thread->set_signal(0);
    }
    else if (sigcont)
    {
        // if we had to force the thread out of a stopped
        // state, "hide" the SIGCONT that we sent

        thread->set_signal(SIGSTOP);
        thread->set_stopped_by_debugger(true);
    }

    init_linker_events(*thread);
    init_thread_agent();

    // put the event back, so that the main event loop picks it
    debugger().queue_event(thread);
}

////////////////////////////////////////////////////////////////
void LinuxLiveTarget::kill_threads(
    pid_t               pid,
    const ThreadList&   threads)
{
    if (sys::uses_nptl())
    {
        dbgout(0) << "killing process " << pid << endl;
        XTrace::kill(pid, SIGKILL);
    }
    else
    {
        ThreadList::const_iterator i = threads.begin();
        for (; i != threads.end(); ++i)
        {
            sys::kill_thread((*i)->lwpid(), SIGKILL, nothrow);
        }
    }
}


////////////////////////////////////////////////////////////////
void LinuxLiveTarget::detach_internal()
{
    pid_t processID = 0;

    if (Process* proc = process())
    {
        if (proc->origin() == ORIGIN_DEBUGGER)
        {
            processID = proc->pid();
        }
    }
    resume_all_threads();
    stop_all_threads();

    ThreadList threadList(threads_begin(), threads_end());

    vector<RefPtr<Thread> >::iterator i = threadList.begin();
    for (; i != threadList.end(); ++i)
    {
        const RefPtr<Thread>& thread = *i;
        thread->detach();
        remove_thread(thread);
    }

    if (processID /* && size() */)
    {
        kill_threads(processID, threadList);
    }
}


////////////////////////////////////////////////////////////////
void LinuxLiveTarget::detach(bool no_throw)
{
    dbgout(0) << __func__ << endl;

    try
    {
        detach_internal();
    }
    catch (const exception& e)
    {
        if (no_throw)
        {
            dbgout(0) << __func__ << ": " << e.what() << endl;
        }
        else
        {
            throw;
        }
    }
}


////////////////////////////////////////////////////////////////
bool
LinuxLiveTarget::set_status_after_stop (

    const RefPtr<ThreadImpl>&   tptr,
    int                         status,
    bool                        queueEvents)
{
    const int eventType = (status >> 16);
    if (eventType)
    {
        handle_extended_event(*tptr, eventType);
        status &= 0xffff;
    }
    tptr->set_status(status);

    if (!set_stopped(tptr, true))
    {
        if (queueEvents && !tptr->is_event_pending())
        {
            debugger().queue_event(tptr);
        }
    }
// todo: the result seems to be ignored, remove this
    return eventType == PTRACE_EVENT_EXIT;
}


////////////////////////////////////////////////////////////////
bool LinuxLiveTarget::event_requires_stop(Thread* thread)
{
    assert(thread);
    const int sig = thread->signal();

    const bool result = 
        !check_extended_event(thread) && debugger().signal_policy(sig)->stop();

    dbgout(1) << __func__ << ": " << thread->lwpid() << "=" << result << endl;
    return result;
}


////////////////////////////////////////////////////////////////
void
LinuxLiveTarget::collect_threads_to_stop(
    ThreadImplMap&  threadsToStop,
    Thread*         threadToSkip
    )
{
    const iterator threadsEnd = threads_end();

    for (iterator i = threads_begin(); i != threadsEnd; ++i)
    {
        if (i->get() == threadToSkip)
        {
            continue;
        }
        if (is_deletion_pending(*i))
        {
            dbgout(0) << (*i)->lwpid() << ": Deletion pending" << endl;
            continue;
        }

        RefPtr<ThreadImpl> thread = interface_cast<ThreadImpl>(*i);

        assert(!(*i)->is_exiting());
        assert(!thread_finished(**i));

        const Runnable::State state = get_thread_state(*CHKPTR(thread));
        switch(state)
        {
        case Runnable::UNKNOWN:
            assert(!is_deletion_pending(thread));
            dbgout(0) << thread->lwpid() << ": gone?" << endl;
            break;

        case Runnable::TRACED_OR_STOPPED:
            if (!thread->has_resumed())
            {
                dbgout(0) << thread->lwpid() << ": already stopped" << endl;
                thread->set_stop_expected(false);
                break;
            }
            // fall through

        default:
            {
                assert(i->get() != threadToSkip);

                const pid_t lwpid = thread->lwpid();
                dbgout(0) << lwpid << " state=" << state
                          << " (" << (char)state << ")" << endl;

                threadsToStop.insert(make_pair(lwpid, thread));
            }
            break;
        }
    }
}


/**
 * Stop all threads but current (if not NULL)
 *
 */
bool LinuxLiveTarget::stop_all_threads(Thread* current)
{
    const bool queueEvents = (current != NULL);

    // Sending a SIGSTOP is asynchronous: make sure that in
    // the offchance of a thread being created while we are
    // in the scope of stopping everyone, the debugger does not
    // allow it to resume
    Temporary<bool> setFlag(resumeNewThreads_, false);

    // a map of threads that we expect to reap with waitpid
    ThreadImplMap threads;

    collect_threads_to_stop(threads, current);

 /* http://www-128.ibm.com/developerworks/linux/library/l-threading.html
   "Since NPTL is POSIX compliant, it handles signals on a
   per-process basis; getpid() returns the same process ID for
   all the threads. For example, if a signal SIGSTOP is sent, the
   whole process would stop; " */

    /* if (sys::uses_nptl())
    {
        if (Process* proc = process())
        {
            if (threads.empty())
            {
                return true;
            }
            sys::kill(proc->pid(), SIGSTOP);
        }
    }
    else */
    {
        ThreadImplMap::const_iterator i = threads.begin();
        
        for (; i != threads.end(); ++i)
        {
            dbgout(0) << __func__ << ": " << i->first << endl;
            
            if (i->second->is_stop_expected())
            {
                dbgout(1) << __func__ << ": stop pending" << endl;
                // we have already sent it a SIGSTOP which has yet  
                // to be delivered
                continue;
            }

            sys::kill_thread(i->first, SIGSTOP);
        }
    }
    ThreadImpl* currentImpl = interface_cast<ThreadImpl*>(current);

    wait_for_threads_to_stop(currentImpl, threads, queueEvents);

    return true;
}



////////////////////////////////////////////////////////////////
void LinuxLiveTarget::wait_for_threads_to_stop (

    ThreadImpl*             current,
    const ThreadImplMap&    threads,
    bool                    queueEvents)

{
    dbgout(2) << __func__ << ": " << threads.size() << endl;

    for (size_t count = 0; count != threads.size(); )
    {
        int status = 0;
        pid_t lwpid = 0;

        // check that map of unhandled events to see if any of
        // the threads that we are waiting for has already stopped
        if (UnhandledMap* umap = debugger().unhandled_map())
        {
            for (UnhandledMap::iterator i = umap->begin();
                 i != umap->end();
                 ++i)
            {
                if (threads.find(i->first) != threads.end())
                {
                    lwpid = i->first;
                    status = i->second;
                    umap->erase(i);
                    break;
                }
            }
        }

        if (!lwpid)
        {
            lwpid = XTrace::waitpid(-1, &status, (__WALL | WUNTRACED));

            if ((lwpid < 0) && (errno == ECHILD))
            {
                break;
            }
        }
        dbgout(2) << "STOPPED: " << lwpid << " " << hex << status << dec << endl;

        ThreadImplMap::const_iterator i = threads.find(lwpid);

        if (i != threads.end())
        {
            set_status_after_stop(i->second, status, queueEvents);
            ++count;
        }
        else
        {
            // save the event, to be handled at a later time
            if (RefPtr<Target> target = debugger().find_target(lwpid))
            {
                ThreadImpl* t = interface_cast<ThreadImpl*>(target->get_thread(lwpid));
                assert(t);

                if (t)
                {
                #if 0
                    t->set_status(status);
                    debugger().queue_event(t);
                #else
                    set_status_after_stop(t, status, queueEvents);
                #endif
                    ++count;
                }
            }
            else
            {
            #if DEBUG
                clog << __func__ << ": THREAD NOT FOUND " << lwpid << endl;
            #endif
                assert(!WIFEXITED(status));
                debugger().save_lwpid_and_status(lwpid, status);
            }
        }
    }
    dbgout(2) << __func__<< ": DONE" << endl;
}


////////////////////////////////////////////////////////////////
Runnable::State LinuxLiveTarget::get_thread_state(ThreadImpl& thread)
{
    Runnable::State state = Runnable::UNKNOWN;
    try
    {
        thread.refresh();
        state = thread.runstate();
    }
    catch (const SystemError& e)
    {
        if (e.error() != ENOENT)
        {
            throw;
        }
    }
    if (state == Runnable::UNKNOWN)
    {
        debugger().cleanup(thread);
    }
    return state;
}


////////////////////////////////////////////////////////////////
void LinuxLiveTarget::stop_async()
{
    if (enum_threads())
    {
        pid_t pid = 0;

        if (Process* proc = this->process())
        {
            pid = proc->pid();
        }
        else
        {
            pid = (*threads_begin())->lwpid();
        }
        sys::kill(pid, SIGSTOP);

        dbgout(0) << __func__ << ": signal sent" << endl;
    }
}


////////////////////////////////////////////////////////////////
size_t LinuxLiveTarget::cleanup(Thread& thread)
{
    if (get_thread(thread.lwpid()))
    {
        remove_thread(&thread);
    }
    return size();
}


////////////////////////////////////////////////////////////////
size_t LinuxLiveTarget::resume_all_threads()
{
    size_t resumedCount = 0;
    bool stepped = false;

    for (iterator i = threads_begin(); i != threads_end(); ++i)
    {
        ThreadImpl& thread = interface_cast<ThreadImpl&>(**i);

        if (thread.has_resumed())
        {
            const Runnable::State state = get_thread_state(thread);

            switch (state)
            {
            case Runnable::RUNNING:
                ++resumedCount;
                continue;

            case Runnable::UNKNOWN:
                clog << __func__ << ": " << thread.lwpid();
                clog << ": unknown state " << int(state) << endl;

                continue;

            default:
                break;
            }
        }

        try
        {
            if (thread.resume(&stepped))
            {
                ++resumedCount;
            }
        }
        catch (const exception& e)
        {
            dbgout(1) << __func__ << ": " << e.what() << endl;
        }
    }
    // If at least one thread has resumed as a result of stepping
    // through a line of high-level code, then we "hide" the event
    // by pretending no thread has resumed.
    //
    // This in turn (see debugger_base.cpp) avoids flooding the
    // plug-ins with unnecessary "on_program_resumed" notifications.

    return stepped ? 0 : resumedCount;
}


////////////////////////////////////////////////////////////////
RefPtr<SymbolMap> LinuxLiveTarget::read_symbols()
{
    SymbolTableEvents* events = debugger().symbol_table_events();

    return read_symbols_from_process(*CHKPTR(process()), *CHKPTR(events));
}


////////////////////////////////////////////////////////////////
void LinuxLiveTarget::handle_event(Thread* thread)
{
    if (!thread)
    {
        assert(false);
        return;
    }
    bool ignoreEvent = false;

    // check for clone(), fork(), or exec() events
    if (const int eventType = (thread->status() >> 16))
    {
        handle_extended_event(*thread, eventType);

        int signum = thread->signal();
        if ((signum == 0) || (signum == SIGTRAP))
        {
            ignoreEvent = true; // no further handling needed
        }
    }
    if (!ignoreEvent)
    {
        init_linker_events(*thread);

        // check for ignored signals
        if (thread_stopped(*thread))
        {
            const int sig = thread->signal();
            if (!debugger().signal_policy(sig)->stop())
            {
                assert(sig != SIGTRAP);
                ignoreEvent = true;
            }
            if (!debugger().signal_policy(sig)->pass())
            {
                ignoreEvent = true;
                // ignore signal (will not be re-delivered)
                thread->set_signal(0);
            }
        }
    }
    if (!ignoreEvent)
    {
        // finally, invoke base class implementation
        UnixTarget::handle_event(thread);
    }
}


/**
 * This is a strange hack that I came up with after some
 * experimentation: it seems that early during initializing
 * libthead_db, several threads share the thr.th_unique
 * value, and in that case the results of ps_pread are bogus.
 *
 * I memorize the th_unique of the "main" thread, to test
 * for this case.
 */
void
LinuxLiveTarget::cache_main_thread_unique(const td_thrhandle_t& thr, pid_t lwpid)
{
    if (mainThreadUnique_ == 0)
    {
        if (Process* proc = process())
        {
            if (proc->pid() == lwpid)
            {
                mainThreadUnique_ = thr.th_unique;
            }
        }
    }
}


/**
 * This method is called when the ThreadAgent iterates over threads
 */
void LinuxLiveTarget::on_thread(

    const td_thrhandle_t*   thr,
    const td_thrinfo&       info)
{
    assert(!is_deletion_pending(info.ti_lid));
    assert(thr);

    thread_t tid = info.ti_tid;
    dbgout(0) << __func__ << ": " << info.ti_lid << endl;

    if (thr)
    {
        cache_main_thread_unique(*thr, info.ti_lid);

        if (thr->th_unique == mainThreadUnique_)
        {
            tid = 0;
        }
    }
    assert(info.ti_lid);

    if (Thread* thread = get_thread(info.ti_lid))
    {
        if (tid)
        {
            ThreadImpl& impl = interface_cast<ThreadImpl&>(*thread);
            impl.set_thread_id(tid);
        }
    }
    else if (Thread* thread = debugger().get_thread(info.ti_lid))
    {
        // thread belongs to another process
        assert(thread->target() != this);

        if (tid && !thread->thread_id())
        {
            ThreadImpl& impl = interface_cast<ThreadImpl&>(*thread);
            impl.set_thread_id(tid);
        }
    }
    else
    {
        cout << "New thread: " << tid << " lwpid=" << info.ti_lid << endl;

        int status = 0;
        try
        {
            sys::ptrace(PTRACE_ATTACH, info.ti_lid, 0, 0);
            sys::waitpid(info.ti_lid, &status, __WCLONE | WUNTRACED);

            assert((status >> 16) == 0);
        }
        catch (const exception& e)
        {
            // Note that an "Operation not permitted" error is normal
            // with newer kernels (2.5.46); we attempt to attach just
            // to support older kernels (2.4.x)
            dbgout(1) << __func__ << ": " << e.what() << " (benign)" << endl;
        }
        Super::on_thread(thr, info);

    /*  If a zero status is passed to create_thread, it will internally
        call ThreadImpl::wait_update_status, which now is intelligent
        enough to lookup a map of unhandled events -- in case that
        waitpid returned info.ti_lid BEFORE the PTRACE_EVENT_CLONE.

        Calling waitpid like here below may just hang.

        if (!status)
        {
            sys::waitpid(info.ti_lid, &status, __WCLONE);
            assert(status);
        }
     */

        RefPtr<ThreadImpl> t = create_thread(tid, info.ti_lid, status);

    #ifdef DEBUG
        dbgout(0) << "| " << __func__ << ": status=" << hex << status;
        dbgout(0) << dec << endl;
        dbgout(0) << "| " << __func__ << ": signal=" << t->signal();
        dbgout(0) << endl;

        if (!resumeNewThreads_) // fails otherwise
        {
            addr_t pc = t->program_count();
            dbgout(0) << __func__ << ": program_count=" << pc << endl;

            assert(pc != get_event_addr(TD_CREATE));
            assert(pc != get_event_addr(TD_DEATH));
        }
    #endif  // DEBUG
    }
}


////////////////////////////////////////////////////////////////
bool
LinuxLiveTarget::write_register(Register& r, const Variant& var)
{

    Reg<>& reg = interface_cast<Reg<>&>(r);

    pid_t pid = 0;
    const size_t off = reg.offset();
    if (off == (size_t)-1)
    {
        return false;
    }

    if (RefPtr<Thread> thread = reg.thread())
    {
        pid = thread->lwpid();
        dbgout(0) << __func__ << ": offset=" << off << endl;

        try
        {
            switch (reg.type())
            {
            case REG_USER:
                sys::ptrace(PTRACE_POKEUSER, pid, off, var.uint64());
                break;

            case REG_FPUX:
                if (Variant* v = r.value())
                {
                    user_fpxregs_struct fpregs;
                    sys::get_regs(pid, fpregs);

                    v->copy(&var, false);
                    char* addr = (char*)&fpregs + off;
                    memcpy(addr, v->data(), v->size());
                    sys::set_regs(pid, fpregs);
                }
                break;
            }
            return true; // success
        }
        catch (const SystemError& e)
        {
            if (e.error() != EIO)
            {
                throw;
            }
        }
    }
    return false;
}


/*
Starting at the address pointed to by sp, the program should be able
to find its arguments, environment variables, and auxiliary vector.
Here's what the stack looks like on a 32-bit system:
  sp:           argc
  sp+4:         argv[0]
  ...
  sp+4*argc:    argv[argc-1]
  sp+4+4*argc:  NULL
  sp+8+4*argc:  envp[0]
  ...
                NULL
*/
static void read_aux_vect(Thread& thread, reg_t sp, AuxVect& vect)
{
    try
    {
        word_t var = 0;
        word_t ptr = sp;
        do
        {
            thread_read(thread, ptr, var);
            Platform::inc_word_ptr(thread, ptr);
        } while (var);

        // skip the environment variables
        do
        {
            thread_read(thread, ptr, var);
            Platform::inc_word_ptr(thread, ptr);
        } while (var);

        const bool dump = env::get_bool("ZERO_DUMP_AUX_VECT");
        if (dump)
        {
            clog << "----- aux_vect -----\n";
        }
        // read the aux vector
        pair<word_t, word_t> entry;

        for (;; Platform::inc_word_ptr(thread, ptr))
        {
            thread_read(thread, ptr, entry.first);
            if (entry.first == 0)
            {
                break;
            }
            Platform::inc_word_ptr(thread, ptr);

            thread_read(thread, ptr, entry.second);

            if (dump)
            {
                clog << " [" << entry.first << "]="
                             << entry.second << "\n";
            }
            vect.push_back(entry);
        }
    }
    catch (...)
    {
    }
}



////////////////////////////////////////////////////////////////
AuxVect LinuxLiveTarget::aux_vect() const
{
    AuxVect v;
    if (readingAuxVect_)
    {
        return v;
    }
    Temporary<bool> setFlag(readingAuxVect_, true);

    if (Process* proc = this->process())
    {
        if (Thread* thread = proc->get_thread(DEFAULT_THREAD))
        {
            read_aux_vect(*thread, thread->stack_start(), v);
        }
    }
    return v;
}


////////////////////////////////////////////////////////////////
VirtualDSO* LinuxLiveTarget::read_virtual_dso() const
{
    VirtualDSO* result = 0;
    try
    {
        if (process())
        {
            addr_t addr = 0;
            size_t sz = 0;

            AuxVect v = aux_vect();

            if (get_sysinfo_ehdr(v, addr, sz))
            {
                ostringstream mem;
                // read one page from /proc/PID/mem
                mem << procfs_root() << process()->pid() << "/mem";

                auto_fd fd(open(mem.str().c_str(), O_RDONLY));
                if (fd.get())
                {
                    result = new VirtualDSO(fd.get(), addr, sz);

                    dbgout(0) << __func__ << ": " << result << endl;
                }
            }
        }
    }
    catch (const exception& e)
    {
        dbgout(0) << __func__ << ": " << e.what() << endl;
    }
    return result;
}


////////////////////////////////////////////////////////////////
void
LinuxLiveTarget::write_register(Thread& thread, size_t n, reg_t v)
{
    const pid_t pid = thread.lwpid();

#ifdef HAVE_PTRACE_POKEUSER
    sys::ptrace(PTRACE_POKEUSER, pid, n * sizeof(reg_t), v);
#else
    reg r;
    sys::ptrace(PT_GETREGS, pid, (addr_t)&r, 0);
    GENERAL_REG(r, n) = v;
    sys::ptrace(PT_SETREGS, pid, (addr_t)&r, 0);
#endif
}


////////////////////////////////////////////////////////////////
bool LinuxLiveTarget::read_register(
    const Thread&   thread,
    int             nreg,
    bool            useFrame,
    reg_t&          rg) const
{
    if (nreg >= 0)
    {
        if (useFrame)
        {
            return LinuxTarget::read_register(thread, nreg, useFrame, rg);
        }
        const size_t offset = nreg * sizeof (reg_t);
        rg = sys::ptrace(PTRACE_PEEKUSER, thread.lwpid(), offset, 0);
    }
    else // hack, refer to fpxregs by negative offset
    {
        assert(static_cast<size_t>(-nreg) < sizeof (user_fpxregs_struct));

        sys::get_fpxregs(thread.lwpid(), fpxregs_);

        rg = (reg_t)(reinterpret_cast<char*>(&fpxregs_) - nreg);
    #ifdef __x86_64__
        *(long double*)rg = *(double*)rg;
    #endif
        dbgout(0) << __func__ << ": " << *(long double*)rg << endl;
    }

    dbgout(0) << __func__ << "(" << nreg << ")=" << (void*) rg << endl;

    return true;
}


////////////////////////////////////////////////////////////////
void LinuxLiveTarget::read_environment(SArray& env) const
{
    if (Process* proc = process())
    {
        ostringstream fname;
        fname << procfs_root() << proc->pid() << "/environ";

        env::read(fname.str(), env);
    }
}


////////////////////////////////////////////////////////////////
static inline auto_ptr<istream>
get_ifstream (
    const Target&   target,
    const string&   filename )
{
    auto_ptr<istream> inp(target.get_ifstream(filename.c_str()));

    if (!inp.get() || !*inp)
    {
        // ENOENT may occur when the thread is about to exit
        if (errno == ENOENT)
        {
            return auto_ptr<istream>();
        }

        throw SystemError(errno, __func__ + (": " + filename), false);
    }
    return inp;
}


////////////////////////////////////////////////////////////////
// Read real, effective and saved uif from /proc/<pid>/status
static void
read_uids( 
    const Target&   target,
    pid_t           lwpid,
    RunnableState&  state )
{
    ostringstream fn;
    fn << "/proc/" << lwpid << "/status";

    auto_ptr<istream> inp = get_ifstream(target, fn.str());

    if (inp.get() == NULL)
    {
        return;
    }

    string key;
    while ( *inp >> key )
    {
        if (key == "Uid:")
        {
            *inp >> state.ruid_;
            *inp >> state.euid_;
            *inp >> state.suid_;
            break;
        }
    }
}


////////////////////////////////////////////////////////////////
bool
LinuxLiveTarget::read_state(
    pid_t           lwpid,
    RunnableState&  state,
    string&         comm
    ) const
{
    ostringstream fn;
    fn << "/proc/" << lwpid << "/stat";
    auto_ptr<istream> in(::get_ifstream(*this, fn.str()));

    if (in.get() == NULL)
    {
        return false;
    }

#ifdef DEBUG
    //system(("ls -l " + fn.str()).c_str());
 #define EXTRACT(x) extract(*in, x, #x, __LINE__)
#else
 #define EXTRACT(x) while (!(*in >> x)) return false;
#endif
#define EXTRACT_AND_IGNORE(type, f) { type f; EXTRACT(f); }
    pid_t pid = 0;
    EXTRACT(pid);
    if (pid == 0)
    {
        return false;
    }
    else if (state.lwpid_ == 0)
    {
        state.lwpid_ = pid;
    }
    else
    {
        assert(state.lwpid_ == pid);
    }

    stringbuf buf;
    if (!in->get(buf, ')'))
    {
        throw SystemError(__func__);
    }
    comm = buf.str();
    while (comm[0] == '(' || comm[0] == ' ')
        comm = comm.substr(1);

    EXTRACT_AND_IGNORE(string, paran);

    EXTRACT(state.state_);
    EXTRACT(state.ppid_);
    EXTRACT(state.gid_);

    EXTRACT(state.session_);
    EXTRACT(state.tty_);
    EXTRACT(state.tpgid_);
    EXTRACT(state.flags_);

    EXTRACT_AND_IGNORE(long unsigned, minflt);
    EXTRACT_AND_IGNORE(long unsigned, cminflt);
    EXTRACT_AND_IGNORE(long unsigned, majflt);
    EXTRACT_AND_IGNORE(long unsigned, cmajflt);

    EXTRACT(state.usrTicks_);  // jiffies in user mode
    EXTRACT(state.sysTicks_);  // jiffies in system moe

    EXTRACT_AND_IGNORE(long, cutime);
    EXTRACT_AND_IGNORE(long, cstime);
    EXTRACT_AND_IGNORE(long, priority);
    EXTRACT_AND_IGNORE(long, nice);
    EXTRACT_AND_IGNORE(long, hardcoded);

    // time in jiffies  before the  next  SIGALRM  is
    // sent to the process due to an interval timer.
    EXTRACT_AND_IGNORE(long, itrealvalue);

    // The time in jiffies the process started after system boot.
    EXTRACT_AND_IGNORE(long unsigned, starttime);

    EXTRACT(state.vmemSize_);  // virtual mem size in bytes

    EXTRACT_AND_IGNORE(long, rss); // resident set size
    EXTRACT_AND_IGNORE(unsigned long long, rlim);
    EXTRACT_AND_IGNORE(unsigned long, startcode);
    EXTRACT_AND_IGNORE(unsigned long, endcode);

    EXTRACT(state.stackStart_);

    // ignore the rest of the file
#undef EXTRACT
#undef EXTRACT_AND_IGNORE

    state.state_ = toupper(state.state_);

    read_uids( *this, lwpid, state );
    return *in;
}


////////////////////////////////////////////////////////////////
bool LinuxLiveTarget::read_state(
    const Thread&   thread,
    RunnableState&  state
    ) const
{
    string comm;
    return read_state(thread.lwpid(), state, comm);
}


////////////////////////////////////////////////////////////////
static void concat(const std::string& src, std::string& dest)
{
    if (!src.empty())
    {
        if (!dest.empty())
        {
            dest += ' ';
        }
        bool quote = false;
        if (src.find_first_of(" \t") != src.npos)
        {
            quote = true;
            dest += '"';
        }
        dest += src;
        if (quote)
        {
            dest += '"';
        }
    }
}


////////////////////////////////////////////////////////////////
const string& LinuxLiveTarget::command_line() const
{
    if (commandLine_.empty())
    {
        if (Process* proc = process())
        {
            ostringstream buf;
            buf << procfs_root() << proc->pid() << "/cmdline";
            ifstream f(buf.str().c_str());
            string tmp, cmd;
            char c;
            while (f >> noskipws >> c)
            {
                if (c)
                {
                    tmp += c;
                }
                else
                {
                    concat(tmp, cmd);
                    tmp.clear();
                }
            }
            concat(tmp, cmd);

            if (cmd.empty())
            {
                // todo: use the process' name here?
            }
            else
            {
                commandLine_ = cmd;
            }
        }
    }
    return commandLine_;
}



////////////////////////////////////////////////////////////////
Target::Kind LinuxLiveTarget::kind() const
{
    if (kind_ == K_UNKNOWN)
    {
        ELF::Binary elf(CHKPTR(process_name())->c_str());
        switch (elf.header().klass())
        {
        case ELFCLASS32:
            kind_ = K_NATIVE_32BIT;
            break;

        case ELFCLASS64:
            kind_ = K_NATIVE_64BIT;
            break;
        }
    }
    return kind_;
}


////////////////////////////////////////////////////////////////
pid_t
LinuxLiveTarget::get_signal_sender_pid(const Thread& thread) const
{
    return ::get_signal_sender_pid(thread.lwpid());
}


////////////////////////////////////////////////////////////////
string LinuxLiveTarget::thread_name(pid_t lwpid) const
{
    string comm;
    RunnableState state(lwpid);

    read_state(lwpid, state, comm);

    return comm;
}

////////////////////////////////////////////////////////////////
void LinuxLiveTarget::update_threads_info()
{
    iterate_threads(*this);
}

////////////////////////////////////////////////////////////////
bool LinuxLiveTarget::is_thread(pid_t pid) const
{
    ostringstream pidstr;
    pidstr << pid;

    const Directory ls(procfs_root(), pidstr.str().c_str());
    bool result = ls.empty();

#if DEBUG
    clog << __func__ << "(" << pid << ")=" << result << endl;
#endif
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
