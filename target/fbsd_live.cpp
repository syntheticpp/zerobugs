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
#include <fstream>
#include <iostream>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <sys/wait.h>
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "zdk/zero.h"
#include "dharma/exec_arg.h"
#include "dharma/process_name.h"
#include "dharma/sigutil.h"
#include "dharma/syscall_wrap.h"
#include "debug_regs_386.h"
#include "debugger_base.h"
#include "fbsd_live.h"
#include "process.h"
#include "symbolz/private/symbol_map_impl.h"
#include "target_factory.h"
#include "thread.h"
#include "unix_exec.h"
#include "utility.h"
#include "x86.h"

using namespace std;


namespace
{
    static RefPtr<Target> create(debugger_type& debugger)
    {
        return new FreeBSDLiveTarget(debugger, __WORDSIZE);
    }
}


void init_live_target()
{
    TheTargetFactory::instance().register_target(
        TargetFactory::FreeBSD,
        __WORDSIZE,
        true,
        string(),
        create);
}


////////////////////////////////////////////////////////////////
FreeBSDLiveTarget::FreeBSDLiveTarget(debugger_type& dbg,
                                     unsigned wordSize)
    : Super(dbg)
    , resumeNewThreads_(false)
{
}


////////////////////////////////////////////////////////////////
Thread* FreeBSDLiveTarget::exec(const ExecArg& arg, const char* const* env)
{
    dbgout(0) << "FreeBSDLiveTarget::exec" << endl;

    RefPtr<Thread> thread = Unix::exec(*this, arg, env);

    assert(thread);
    assert(thread->ref_count() > 1);

    return thread.get();
}


////////////////////////////////////////////////////////////////
void FreeBSDLiveTarget::attach(pid_t pid)
{
    assert(enum_threads() == 0); // pre-condition

    RunnableImpl task(pid, this);
    if (task.runstate() == Runnable::ZOMBIE)
    {
        throw runtime_error("cannot attach to defunct process");
    }
    dbgout(0) << __func__ << ": state=" << task.runstate() << endl;

    sys::ptrace(PTRACE_ATTACH, pid, 0, 0);

    bool cont = false;
    if (task.runstate() == Runnable::TRACED_OR_STOPPED)
    {
        sys::kill(pid, SIGCONT);
        cont = true;
    }

    int status = 0;
    sys::waitpid(pid, &status, __WALL);

    init_process(pid, NULL, ORIGIN_SYSTEM);
    assert(process());

    init_symbols();
    assert(symbols());

    init_thread_agent();
    RefPtr<ThreadImpl> thread(create_thread(0, pid, status));

    if (this->kind() == K_NATIVE_32BIT)
    {
        set_word_size(32);
    }

    if (cont)
    {
        // if we had to force the thread out of a stopped
        // state, "hide" the SIGCONT that we sent
        thread->set_signal(SIGSTOP);
        thread->set_stopped_by_debugger(true);
    }
    init_linker_events(*thread);
    //init_thread_agent();

    // put the event back, so that it will be picked
    // by the main event loop
    debugger().queue_event(thread);

#ifdef DEBUG
    clog << "***** " << __PRETTY_FUNCTION__ << " *****\n";
#endif
}


////////////////////////////////////////////////////////////////
void FreeBSDLiveTarget::detach(bool no_throw)
{
    if (is_attached())
    {
        // todo
        // assert(false);
    }
}


////////////////////////////////////////////////////////////////
pid_t FreeBSDLiveTarget::pid_to_lwpid(pid_t pid) const
{
    return pid;
    ptrace_lwpinfo pl = { 0 };
    try
    {
        do
        {
            if (sys::ptrace(PT_LWPINFO, pid, (addr_t)&pl, sizeof pl) < 0)
            {
                return -1;
            }
            assert(pl.pl_lwpid);
/* #ifdef DEBUG
            clog << __func__ << " lwpid=" << pl.pl_lwpid << ", ";
            clog << "event=" << pl.pl_event << ", ";
            clog << "flags=" << pl.pl_flags << endl;
#endif */
        } while (0 /*pl.pl_event != PL_EVENT_SIGNAL*/);
    }
    catch (const exception& e)
    {
#ifdef DEBUG
        clog << __func__ << ": " << e.what() << endl;
#endif
        pl.pl_lwpid = pid;
    }
    return pl.pl_lwpid;
}


////////////////////////////////////////////////////////////////
RefPtr<SymbolMap> FreeBSDLiveTarget::read_symbols()
{
    SymbolTableEvents* events = debugger().symbol_table_events();

    RefPtr<SymbolMapImpl> symbols =
        new SymbolMapImpl(*CHKPTR(process()), *CHKPTR(events));

    symbols->read();
    return symbols;
}


////////////////////////////////////////////////////////////////
RefPtr<ProcessImpl>
FreeBSDLiveTarget::new_process(pid_t pid,
                               const ExecArg* arg,
                               ProcessOrigin origin
                              )
{
    const std::string* cmdline = NULL;

    if (arg)
    {
        cmdline = &arg->command_line();
    }

    return new ProcessImpl( *this,
                            pid,
                            origin,
                            cmdline,
                            get_process_name(pid).c_str());
}


////////////////////////////////////////////////////////////////
RefPtr<Thread>
FreeBSDLiveTarget::new_thread(long id, pid_t lwpid, int status)
{
    assert(this->process());
    dbgout(0) << __func__ << ": id=" << id << " lwpid=" << lwpid << endl;
    return create_thread(id, lwpid, status);
}


////////////////////////////////////////////////////////////////
bool FreeBSDLiveTarget::stop_all_threads(Thread* thread)
{
    init_thread_agent();

    vector<pid_t> lwp;      // light-weight process IDs
    const size_t n = get_lwp_list(lwp);

    for (size_t i = 0; i != n; ++i)
    {
        unsigned long id = lwpid_to_tid(lwp[i]);
        if (thread && thread->thread_id() == id)
        {
            continue;
        }

        RefPtr<ThreadImpl> tptr = interface_cast<ThreadImpl*>(get_thread(0, id));

        int status = 0;
        static const int wflags = __WALL | WNOHANG | WUNTRACED;

        // has the thread stopped already?
        pid_t rpid = tptr->wait_internal(&status, wflags, true);

        if (rpid < 0)
        {
            assert(errno == ECHILD);

    #ifdef DEBUG
            clog << __func__ << ": " << tptr->lwpid();
            clog << ": " << strerror(errno) << endl;
    #endif
            debugger().cleanup(*tptr);
            continue;
        }
        else if (rpid > 0)
        {
            tptr->set_status(status);
        }
        else if (!tptr->update(nothrow))
        {
            if (errno == EPERM)
            {
                continue;
            }
            // debugger().stop_thread(tptr);
            // TODO
        }

        if (thread
            && !tptr->is_event_pending()
            && !tptr->is_stopped_by_debugger())
        {
            debugger().queue_event(tptr);
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////
size_t FreeBSDLiveTarget::resume_all_threads()
{
    size_t resumedCount = 0;// how many threads resumed
    init_thread_agent();

    vector<pid_t> lwp;      // light-weight process IDs
    const size_t n = get_lwp_list(lwp);

    for (size_t i = 0; i != n; ++i)
    {
        thread_t id = lwpid_to_tid(lwp[i]);
        Thread* thread = get_thread(0, id);
        if (!thread)
        {
            continue;
        }
        ThreadImpl& timpl = interface_cast<ThreadImpl&>(*thread);

        try
        {
            if (timpl.resume())
            {
        #ifdef DEBUG
                clog << "thread " << id << " resumed\n";
        #endif
                ++resumedCount;
            }
            continue;
        }
        catch (const exception& e)
        {
            cerr << __func__ << ": " << e.what() << endl;
        }

        // if we got here, timpl.resume() has failed
        if (thread_finished(timpl))
        {
            debugger().cleanup(timpl);
        }
    }
    return resumedCount;
}


////////////////////////////////////////////////////////////////
void FreeBSDLiveTarget::read_memory(
    pid_t       pid,
    SegmentType seg,
    addr_t      addr,
    long*       buf,
    size_t      buflen, // length of buffer in machine words
    size_t*     wordsRead) const
{
    if (wordsRead)
    {
        *wordsRead = 0;
    }
    if (Process* proc = process())
    {
        ptrace_io_desc pt_io = {
            seg == DATA_SEGMENT ? PIOD_READ_D : PIOD_READ_I,
            (void*)addr,
            (void*)buf,
            buflen * sizeof(word_t)
        };
        if (wordsRead)
        {
            ptrace(PT_IO, proc->pid(), (caddr_t)&pt_io, 0);
            assert((pt_io.piod_len % sizeof(word_t)) == 0);
            *wordsRead = pt_io.piod_len / sizeof(word_t);
        }
        else
        {
            sys::ptrace(PT_IO, proc->pid(), (addr_t)&pt_io, 0);
        }
    }
}


////////////////////////////////////////////////////////////////
void FreeBSDLiveTarget::write_memory(
    pid_t       pid,
    SegmentType seg,
    addr_t      addr,
    const long* buf,
    size_t      nwords
)
{
    if (Process* proc = process())
    {
        ptrace_io_desc pt_io = {
            seg == DATA_SEGMENT ? PIOD_WRITE_D : PIOD_WRITE_I,
            (void*)addr,
            (void*)buf,
            nwords * sizeof(word_t) // bytes
        };
        sys::ptrace(PT_IO, proc->pid(), (addr_t)&pt_io, 0);
        assert(pt_io.piod_len == nwords * sizeof(word_t));
    }
}


////////////////////////////////////////////////////////////////
size_t FreeBSDLiveTarget::get_lwp_list(vector<pid_t>& lwps)
{
    if (Process* proc = this->process())
    {
        const register pid_t pid = proc->pid();
        size_t n = sys::ptrace(PT_GETNUMLWPS, pid, 0, 0);
        lwps.resize(n);

        sys::ptrace(PT_GETLWPLIST, pid, (addr_t)&lwps[0], n);
    }
    return lwps.size();
}


////////////////////////////////////////////////////////////////
bool
FreeBSDLiveTarget::map_lwpid(pid_t pid, td_thrhandle_t& th) const
{
    bool result = false;
    if (ta_)
    {
        if (td_ta_map_lwp2thr(ta_.get(), pid, &th) == TD_OK)
        {
            result = true;
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
unsigned long FreeBSDLiveTarget::lwpid_to_tid(lwpid_t pid) const
{
    thread_t tid = 0;
    td_thrhandle_t th = { 0 };

    if (map_lwpid(pid, th))
    {
        td_thrinfo_t info = { 0 };
        if (td_thr_get_info(&th, &info) == TD_OK)
        {
            tid = info.ti_tid;
#ifdef DEBUG
            clog << __func__ << "=" << tid << endl;
#endif
        }
    }
    return tid;
}


////////////////////////////////////////////////////////////////
bool
FreeBSDLiveTarget::map_tid(thread_t id, td_thrhandle_t& th) const
{
    bool result = false;
    if (ta_)
    {
        if (td_ta_map_id2thr(ta_.get(), id, &th) == TD_OK)
        {
            result = true;
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
pid_t FreeBSDLiveTarget::tid_to_lwpid(long tid) const
{
    lwpid_t pid = 0;
    td_thrhandle_t th = { 0 };

    //dbgout(2) << __func__ << ": ta_=" << ta_ << endl;

    if (map_tid(tid, th))
    {
        td_thrinfo_t info = { 0 };
        if (td_thr_get_info(&th, &info) == TD_OK)
        {
            pid = info.ti_lid;
        }
    }
    return pid;
}


////////////////////////////////////////////////////////////////
Thread* FreeBSDLiveTarget::event_pid_to_thread(pid_t pid) const
{
    assert(pid);
#if 0
    if (pid == DEFAULT_THREAD) // by convention
    {
        return threads_.empty() ? NULL : threads_.begin()->second.get();
    }
#endif
    pid_t lwpid = pid_to_lwpid(pid);
    return get_thread(lwpid, unsigned(-1));
}


////////////////////////////////////////////////////////////////
size_t FreeBSDLiveTarget::cleanup(Thread&)
{
    //assert(false);
    // todo
    return 0;
}


////////////////////////////////////////////////////////////////
void FreeBSDLiveTarget::stop_async()
{
    assert(false); // todo
}


////////////////////////////////////////////////////////////////
bool FreeBSDLiveTarget::write_register(Register&, const Variant&)
{
    assert(false); // todo
    return false;
}

////////////////////////////////////////////////////////////////
void FreeBSDLiveTarget::write_register(Thread&, size_t, reg_t)
{
    assert(false); // todo
}


////////////////////////////////////////////////////////////////
bool
FreeBSDLiveTarget::read_register(const Thread&,
                                 int regnum,
                                 bool fromStackFrame,
                                 reg_t& regOut) const
{
    assert(false); // todo
    return false;
}


////////////////////////////////////////////////////////////////
ZObject* FreeBSDLiveTarget::regs(const Thread& thread) const
{
    ZObject* result = thread.regs();
    if (!result)
    {
        RefPtr<GRegs> r(new GRegs);
        sys::get_regs(thread.lwpid(), *r);
        result = thread.regs(r.get());
    }
    return result;
}


////////////////////////////////////////////////////////////////
ZObject* FreeBSDLiveTarget::fpu_regs(const Thread&) const
{
    assert(false);
    return 0;
}


////////////////////////////////////////////////////////////////
word_t
FreeBSDLiveTarget::get_breakpoint_opcode(word_t code) const
{
    return x86_get_breakpoint_opcode(code);
}


////////////////////////////////////////////////////////////////
void
FreeBSDLiveTarget::read_environment(SArray&) const
{
    // todo
}

////////////////////////////////////////////////////////////////
RefPtr<ThreadImpl>
FreeBSDLiveTarget::create_thread(long id, pid_t lwpid, int status)
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
    if (thread->signal() == SIGSTOP)
    {
        thread->set_stopped_by_debugger(true);
    }

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

bool FreeBSDLiveTarget::read_state(const Thread& thread, RunnableState& state) const
{
    const pid_t pid = thread.lwpid();
    assert(pid);
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID | KERN_PROC_INC_THREAD, pid };
    kinfo_proc p = { 0 };
    size_t len = sizeof(p);
    if (sysctl(mib, 4, &p, &len, NULL, 0) == 0 && len == size_t(p.ki_structsize))
    {
        state.state_ = p.ki_stat;
        state.lwpid_ = p.ki_pid;
        state.ppid_ = p.ki_ppid;
        state.gid_ = p.ki_pgid;
        state.session_ = p.ki_sid;
        state.tty_ = p.ki_tdev;
        state.tpgid_ = p.ki_tpgid; // tty process group id

        state.vmemSize_ = p.ki_size;
        state.stackStart_ = uint64_t(p.ki_kstack);
    }
    return false;
}


pid_t FreeBSDLiveTarget::get_signal_sender_pid(const Thread&) const
{
    return -1; // unknown
}


addr_t FreeBSDLiveTarget::setup_caller_frame(Thread& thread, addr_t sp, long pc)
{
    // push the current program counter so that the called
    // function returns here
    Platform::dec_word_ptr(thread, sp);
    thread_poke_word(thread, sp, pc);

    CHKPTR(get_runnable(&thread))->set_stack_pointer(sp);

    return sp;
}


auto_ptr<DebugRegsBase> FreeBSDLiveTarget::get_debug_regs(Thread& thread) const
{
    return auto_ptr<DebugRegsBase>(new DebugRegs386(thread));
}


bool FreeBSDLiveTarget::event_requires_stop(Thread*)
{
    return true;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
