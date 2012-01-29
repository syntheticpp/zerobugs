//
// $Id$
//
// Clasess: RunnableImpl, ThreadImpl
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/config.h"
#include <assert.h>
#include <errno.h>
#include <signal.h>     // SIGTRAP
#ifdef HAVE_UNISTD_H
 #include <unistd.h>
#endif
#ifdef HAVE_SYS_WAIT_H
 #include <sys/wait.h>   // WIFEXITED, etc.
#endif
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include "generic/state_saver.h"
#include "generic/temporary.h"
#include "dharma/directory.h"
#include "dharma/environ.h"
#include "dharma/process_name.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/types.h"
#include "symbolz/public/symbol_map.h"
#include "debugger_base.h"
#include "process.h"
#include "ptrace.h"
#include "ret_symbol.h"
#include "stack_trace.h"
#include "target/jump.h"
#include "target/target.h"
#include "thread.h"
#include "unhandled_map.h"

#define dbgstep() dbgout(1)

#if defined(__PPC__)
 #define RETURN_ADDR_FROM_STACK(thread) (thread).read_register(PT_LNK, false)
#else
 #define RETURN_ADDR_FROM_STACK(thread) thread_peek_stack((thread))
#endif

using namespace std;


/**
 * @return the return address of current function.
 * helper called from resume_stepping
 */
static inline addr_t step_return_addr(ThreadImpl& thread)
{
    static bool heuristics = env::get_bool("ZERO_INLINE_HEURISTICS", 0);

    if (heuristics)
    {
        return thread.ret_addr();
    }
#if __linux__
    // Has clone or fork syscall just executed?
    // (I should check the syscall NR but it differs from 32 to 64 bit,
    // it's a simpler heuristic to check for the short stack trace).
    if (thread.stack_trace_depth() < 2)
    {
        addr_t pc = thread.program_count() - 2;
        size_t count = 0;
        word_t w;
        thread.read_code(pc, &w, 1, &count);
        if ((w & 0xffff) == 0x050f) // syscall?
        {
            return thread.ret_addr();
        }
    }
#endif
    return RETURN_ADDR_FROM_STACK(thread);
}


////////////////////////////////////////////////////////////////
ostream& operator<<(ostream& outs, const Thread& thread)
{
    StateSaver<ios, ios::fmtflags> flags(outs);

    static const int width = 5;

    outs << setw(width) << setfill(' ') << thread.lwpid()
         << ' ' << static_cast<char>(thread.runstate())
         << ' ' << setw(width) << thread.ppid()
         << ' ' << setw(width) << thread.gid()
         << ' ' << setw(12) << thread.thread_id();

    return outs;
}



////////////////////////////////////////////////////////////////
RunnableImpl::RunnableImpl
(
    pid_t       lwpid,
    Target*     target,
    ThreadImpl* thread
)
    : RunnableState(lwpid)
    , target_(target)
    , thread_(thread)
    , singleStep_(false)
{
    assert(thread == NULL || thread->is_live());
    refresh();
}


////////////////////////////////////////////////////////////////
const char* RunnableImpl::name() const
{
    if (name_.is_null())
    {
        if (RefPtr<Target> target = this->target())
        {
            name_ = target->process_name(lwpid_);
        }
        else if (lwpid_)
        {
            name_ = shared_string(get_process_name(std::nothrow, lwpid_));
        }
    }
    return name_ ? name_->c_str() : "";
}


////////////////////////////////////////////////////////////////
Process* RunnableImpl::process() const
{
    if (RefPtr<Target> target = target_.ref_ptr())
    {
        return target->process();
    }
    return NULL;
}

////////////////////////////////////////////////////////////////
void RunnableImpl::refresh()
{
    if (RefPtr<Target> target = this->target())
    {
        if (thread_)
        {
            target->read_state(*thread_, *this);
        }
        else
        {
            ThreadImpl thread(*target, 0, lwpid_);
            target->read_state(thread, *this);
        }
    }
}


////////////////////////////////////////////////////////////////
Runnable::State RunnableImpl::runstate() const
{
    return static_cast<Runnable::State>(state_);
}


////////////////////////////////////////////////////////////////
void RunnableImpl::set_next(Symbol* sym)
{
    if (thread_)
    {
        thread_->set_next(sym);
    }
}


////////////////////////////////////////////////////////////////
void RunnableImpl::set_stepping(bool stepping)
{
    if (thread_)
    {
        thread_->set_stepping(stepping);
    }
}


////////////////////////////////////////////////////////////////
addr_t RunnableImpl::stack_start() const
{
    return stackStart_;
}


////////////////////////////////////////////////////////////////
Thread* RunnableImpl::thread() const
{
    return thread_;
}


////////////////////////////////////////////////////////////////
bool RunnableImpl::resume()
{
    if (thread_)
    {
        return thread_->resume();
    }
    sys::kill(lwpid_, SIGCONT);
    return true;
}


////////////////////////////////////////////////////////////////
typedef EnumCallback2<int, const char*> FileCallback;

size_t RunnableImpl::enum_open_files(FileCallback* cb) const
{
    ostringstream os;

    if (RefPtr<Target> target = target_.ref_ptr())
    {
        os << target->procfs_root();
    }
    else
    {
        os << "/proc/";
    }
    os << this->pid() << "/fd";

    Directory fd(os.str());
    if (cb)
    {
        const Directory::const_iterator end = fd.end();
        Directory::const_iterator i = fd.begin();

        for (; i != end; ++i)
        {
            if (i.short_path() == "." || i.short_path() == "..")
            {
                continue;
            }
            char buf[PATH_MAX + 1] = { 0 };
            const char* unused = realpath((*i).c_str(), buf);
            unused = unused;
            const char* path = strstr(buf, ":[");
            if (path)
            {
                path = strrchr(buf, '/');
            }
            if (path)
            {
                ++path;
            }
            else
            {
                path = buf;
            }

            pid_t pid = strtoul(i.short_path().c_str(), 0, 0);
            cb->notify(pid, path);
        }
    }
    return fd.size();
}


////////////////////////////////////////////////////////////////
void RunnableImpl::set_program_count(addr_t addr)
{
    if (CHKPTR(thread_)->program_count() != addr)
    {
        if (RefPtr<Target> t = target())
        {
            thread_->clear_cached_regs();
            t->set_program_count(*thread_, addr);

            thread_->reset_stack_trace();
        }
    }
}


////////////////////////////////////////////////////////////////
void RunnableImpl::set_result(word_t result)
{
    if (thread_)
    {
        if (RefPtr<Target> t = target())
        {
            t->set_result(*thread_, result);
            thread_->clear_cached_regs();
        }
    }
}


////////////////////////////////////////////////////////////////
void RunnableImpl::set_result64(int64_t result)
{
    if (thread_)
    {
        if (RefPtr<Target> t = target())
        {
            t->set_result64(*thread_, result);
            thread_->clear_cached_regs();
        }
    }
}


////////////////////////////////////////////////////////////////
void RunnableImpl::set_result_double(long double r, size_t size)
{
    if (RefPtr<Target> t = target())
    {
        t->set_result_double(*thread_, r, size);
        thread_->clear_cached_regs();
    }
}


////////////////////////////////////////////////////////////////
void RunnableImpl::set_stack_pointer(addr_t addr)
{
    if (thread_)
    {
        if (RefPtr<Target> t = target())
        {
            t->set_stack_pointer(*thread_, addr);

            thread_->reset_stack_trace();
            thread_->clear_cached_regs();
        }
    }
}


////////////////////////////////////////////////////////////////
bool RunnableImpl::write_register(Register* reg, const Variant* var)
{
    bool result = false;
    if (reg && var)
    {
        if (RefPtr<Target> t = target())
        {
            result = t->write_register(*reg, *var);
            if (result)
            {
                thread_->reset_stack_trace();
                thread_->clear_cached_regs();
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
void RunnableImpl::write_register(size_t n, reg_t v)
{
    if (RefPtr<Target> target = this->target())
    {
        target->write_register(*thread_, n, v);
        thread_->clear_cached_regs();
    }
}


////////////////////////////////////////////////////////////////
void RunnableImpl::set_registers(ZObject* usr, ZObject* fpu)
{
    if (RefPtr<Target> target = this->target())
    {
        target->set_registers(*thread_, usr, fpu);

        thread_->clear_cached_regs();
        thread_->reset_stack_trace();
    }
}


/**
 * Set the single stepping mode of this thread,
 * optionally specifying an action context. The
 * action context may be used later to identify
 * the entity that initiated the single-stepping.
 */
void RunnableImpl::set_single_step_mode(bool mode, ZObject* context)
{
    set_single_step(mode);
    CHKPTR(thread_)->set_single_step_mode(mode, context);
}


/**
 * Execute one single machine instruction
 */
void RunnableImpl::step_instruction()
{
    CHKPTR(thread_)->step_instruction();
}


/**
 * Execute instructions until the current function returns.
 * @note the method is asynchronous; the caller should not
 * expect the debugged function to return when this method
 * completes; rather, the method arranges for the thread to
 * break in the debugger when the current function returns.
 * @note it is up to the implementation if this is achieved
 * by single-stepping or by setting a temporary, internal
 * breakpoint at the return address on the stack.
 */
void RunnableImpl::step_until_current_func_returns()
{
    CHKPTR(thread_)->step_until_current_func_returns();
}



////////////////////////////////////////////////////////////////
void RunnableImpl::freeze(bool)
{
    throw runtime_error("not implemented");
}


////////////////////////////////////////////////////////////////
bool RunnableImpl::is_frozen() const
{
    throw runtime_error("not implemented");
}


////////////////////////////////////////////////////////////////
void RunnableImpl::clear_cached_regs()
{
    if (thread_)
    {
        thread_->clear_cached_regs();
    }
}


/**
 * check for PLT/GOT jump at given program counter
 */
static bool is_plt_jump(Thread& thread, addr_t pc) throw()
{
    bool result = false;
    try
    {
        SymbolMap* symbols = CHKPTR(thread.symbols());

        RefPtr<Symbol> sym = symbols->lookup_symbol(pc);
        result = is_plt_jump(thread, sym.get(), pc);
    }
    catch (...)
    {
    }
    return result;
}


/**
 * Helper class that models a range of instructions to be
 * stepped over (or into).
 */
class ZDK_LOCAL ThreadImpl::StepRange : boost::noncopyable
{
public:
    StepRange(addr_t begin, addr_t end, ThreadImpl& thread)
        : stepInto_(false)
        , inStub_(false)
        , longjump_(false)
        , range_(begin, end)
        , over_(0)
        , retAddr_(thread.ret_addr())
    {
        assert(range_.first);
        assert(range_.second);

        thread.set_call_info();
    }

    addr_t begin() const { return range_.first; }

    addr_t end() const { return range_.second; }

    addr_t stepped_over() const { return over_; }

    void set_stepped_over(addr_t over) { over_ = over; }

    bool step_into() const { return stepInto_; }

    void set_step_into(bool step) { stepInto_ = step; }

    const RefPtr<Symbol>& origin() const { return origin_; }

    bool in_stub() const { return inStub_; }
    void set_in_stub(bool inStub) { inStub_ = inStub; }

    addr_t ret_addr() const { return retAddr_; }

    /**
     * When enabled, check whether we stepped from a part of the
     * code that has an source into a part that doesn't (such as
     * a third-party lib or some compiler-generated trampoline).
     * @return true if we should skip stepping through the code
     * (because we don't have source code for it)
     * The ZERO_SOURCE_STEP environment variable, if set to false,
     * causes the debugger to step into a function even if we
     * do not have source code for it.
     * NOTE: one special case is the first-time calling of functions
     * imported from shared objects, where the dynamic loader/linker
     * fixup code is called via the PLT.
     */
    bool skip_step(Thread& thread, addr_t pc) const
    {
        static const bool check = env::get_bool("ZERO_SOURCE_STEP", true);

        if (!stepInto_)
        {
            return false; // nothing to skip
        }

        bool result = false;

        if (SymbolMap* symbols = thread.symbols())
        {
            if (Debugger* debugger = thread.debugger())
            {
                if (debugger->query_step_over(symbols, pc))
                {
                    result = true;
                }
            }
            if (check && !result)
            {
                RefPtr<Symbol> sym = symbols->lookup_symbol(pc);

                // no source code available at program count?
                if (!sym || sym->line() == 0)
                {
                    // source was available at the place where
                    // we started stepping?
                    if (begin() && !origin_)
                    {
                        origin_ = symbols->lookup_symbol(begin());
                    }
                    result = (origin_.get() && origin_->line());
                    if (result)
                    {
                        if (is_plt_jump(thread, sym.get(), pc))
                        {
                            result = false;
                        }
                    }
                }
            }
        }
        return result;
    }

    bool longjump_called() const
    {
        return longjump_;
    }
    void set_longjump_called(bool flag = true)
    {
        longjump_ = flag;
    }

private:
    typedef std::pair<addr_t, addr_t> Range;

    bool                    stepInto_; // false when stepping over
    bool                    inStub_;
    bool                    longjump_;
    Range                   range_;
    addr_t                  over_;
    const addr_t            retAddr_;
    mutable RefPtr<Symbol>  origin_;
};


//////////////////////////////////////////////////////////////
ThreadImpl::ThreadImpl
(
    Target&         target,
    thread_t        threadID,
    pid_t           pid
)
    : ThreadBase()
    , threadID_(threadID)
    , symbols_(target.symbols())
    , debugger_(&target.debugger())
    , status_(0)
    , eventPending_(0)
    , sender_(-1)
    , flags_(0)
    , callerProgramCount_(0)
    , runnable_(pid, &target, this)
    , nextStackDepth_(0)
{
    debugRegs_ = target.get_debug_regs(*this);
}


///////////////////////////////////////////////////////////////
ThreadImpl::~ThreadImpl() throw()
{
    assert(!deleted_);
    deleted_ = true;
}


////////////////////////////////////////////////////////////////
pid_t ThreadImpl::lwpid() const
{
    pid_t pid = 0;

 //ifdefs are not very decorous, but the extra calls to the
 // target encumber a performance penalty; I may however reconsider
 // this argument when (and if) I support remote and cross-debugging.

#if defined(HAVE_KSE_THREADS)

    if (RefPtr<Target> target = runnable_.target())
    {
        pid = target->tid_to_lwpid(threadID_);
    }
    if (!pid)
#endif
    {
        pid = runnable_.pid();
        assert(pid);
    }
    return pid;
}


////////////////////////////////////////////////////////////////
const char* ThreadImpl::filename() const
{
    if (Process* p = this->process())
    {
        return p->name();
    }
    return runnable_.name();
}


////////////////////////////////////////////////////////////////
addr_t program_count(const Thread& thread)
{
    addr_t pc = thread.program_count();
    const bool inSysCall = thread.is_syscall_pending(pc);

    if (inSysCall)
    {
        if (StackTrace* stack = thread.stack_trace(1))
        {
            if (stack->size())
            {
                pc = stack->frame(0)->program_count();
            }
        }
        else
        {
            pc = RETURN_ADDR_FROM_STACK(thread);
        }
    }
    return pc;
}


////////////////////////////////////////////////////////////////
unsigned long ThreadImpl::thread_id() const
{
    if (!threadID_)
    {
        if (RefPtr<Target> target = runnable_.target())
        {
            threadID_ = target->lwpid_to_tid(lwpid());
        }
    }
    return threadID_;
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_thread_id(thread_t id)
{
    assert(id);

    threadID_ = id;
}


////////////////////////////////////////////////////////////////
addr_t ThreadImpl::ret_addr()
{
    // Use address saved in the stack frame to determine
    // the return address.
    // Normally there should be a call trace with more than
    // one entry, with one exception: when this is called
    // right at program startup (or after a thread just forked)

    if (stack_trace_depth() < 2)
    {
        trace_.reset();
    }

    StackTrace* trace = stack_trace();

    if (trace && trace->size())
    {
        if (Frame* frame = trace->selection())
        {
            if (frame->index() == 0) // top frame?
            {
                if (StackTraceImpl::check_prologue(*this, *frame))
                {
                    // the program counter at frame is within a
                    // piece of code that looks like the prologue
                    // of a C / C++ function
                    const addr_t addr = RETURN_ADDR_FROM_STACK(*this);
                    return addr;
                }
            }
            const size_t nextFrameIndex = frame->index() + 1;

            if (nextFrameIndex < trace->size())
            {
                frame = trace->frame(nextFrameIndex);
                if (frame)
                {
                    dbgout(1) << (void*) frame->program_count() << endl;

                    return frame->program_count();
                }
            }
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////
int ThreadImpl::signal() const
{
    int signum = 0;

    if (WIFSTOPPED(status_))
    {
        signum = WSTOPSIG(status_);
    }
    else if (WIFSIGNALED(status_))
    {
        signum = WTERMSIG(status_);
    }
    if (signum == (SIGTRAP | 0x80))
    {
        signum = SIGTRAP;
    }
    return signum;
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_signal(int sigNum)
{
    if (!WIFSTOPPED(status_))
    {
        status_ = 0x7f;
    }

    status_ &= 0x00ff;
    status_ |= (sigNum << 8);

    assert(signal() == sigNum);
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_status_internal(int status)
{
    Lock<Mutex> lock(mutex_);
    clear_cached_regs();

    status_     = status;
    resumed_    = false;
    sender_     = -1;   // pid of signal sender

    trace_.reset();

    if (RefPtr<Target> target = runnable_.target())
    {
        sender_ = target->get_signal_sender_pid(*this);
    }

    if (signal() == SIGSTOP)
    {
        if (is_stop_expected())
        {
            set_stopped_by_debugger(true);
        }

        set_stop_expected(false);

        dbgout(1) << "reset stop expected" << endl;
    }
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_status(int status)
{
    if (WIFSTOPPED(status))
    {
        runnable_.refresh();
    }

    set_status_internal(status);

    if (RefPtr<Target> target = runnable_.target())
    {
        target->debugger().on_update(*this);
    }
    else
    {
        dbgout(0) << "no target" << endl;
    }
}



////////////////////////////////////////////////////////////////
bool ThreadImpl::update(const std::nothrow_t& nothrow) throw()
{
    bool success = false;
    if (RefPtr<Target> target = runnable_.target())
    {
        success = target->regs(*this);

        if (success)
        {
            try
            {
                int status = 0;

                pid_t rpid = wait_internal(&status, __WALL | WNOHANG, true);

                if (rpid < 0)
                {
                    assert(false); // not expected to fail here
                    success = false;
                }
                else if (rpid > 0)
                {
                    assert(rpid == this->lwpid());
                    DEBUG_TRACE_EVENT(lwpid(), status, PTRACE_EVENT_CLONE);
                    set_status_internal(status);

                    target->debugger().on_update(*this);
                }
            }
            catch (const exception& e)
            {
                cerr << "Failed updating thread: " << e.what() << endl;
            }
            // catch (...) {}
            //   if other type of exceptions are thrown (i.e. not
            //   in the std::exception hierarchy), it is a bug, let
            //   the program terminate
        }
    }

    return success;
}


////////////////////////////////////////////////////////////////
SymbolMap* ThreadImpl::symbols() const
{
    return symbols_.get();
}


////////////////////////////////////////////////////////////////
void ThreadImpl::read_symbols(SymbolTableEvents* events)
{
    assert(events);
    assert(!symbols_);
    assert(process());
#ifndef HAVE_KSE_THREADS
    assert(process()->pid() == this->lwpid());
#endif

    symbols_ = read_symbols_from_process(*process(), *events);
}



////////////////////////////////////////////////////////////////
bool ThreadImpl::single_step_mode() const
{
    return singleStepMode_;
}



////////////////////////////////////////////////////////////////
void ThreadImpl::set_single_step_mode(bool mode, ZObject* context)
{
    singleStepMode_ = mode;

    dbgout(1) << "mode=" << mode << endl;

    if (RefPtr<Target> target = runnable_.target())
    {
        if (mode)
        {
            target->debugger().set_lwpid_step(lwpid());
            if (context)
            {
                actionContext_.reset(context);
            }
        }
        else
        {
            if (!stepRange_.get())
            {
                target->debugger().set_lwpid_step(0);
            }
        }
    }
}


////////////////////////////////////////////////////////////////
StackTrace* ThreadImpl::stack_trace(size_t depth) const
{
    Lock<Mutex> lock(mutex_);
    stack_check_max_depth(depth);

    if (trace_)
    {
        if (trace_->is_unwinding()) // prevent infinite recursion
        {
            return trace_.get();
        }
        if ((trace_->size() < depth) && !trace_->is_complete())
        {
            trace_.reset();
        }
    }
    if (!trace_)
    {
        trace_.reset(new StackTraceImpl(*this));
        trace_->unwind(*this, depth, oldTrace_.get());

        oldTrace_ = trace_;
    }
    return trace_.get();
}


////////////////////////////////////////////////////////////////
size_t ThreadImpl::stack_trace_depth() const
{
    Lock<Mutex> lock(mutex_);
    return trace_ ? trace_->size() : 0;
}


////////////////////////////////////////////////////////////////
void ThreadImpl::reset_stack_trace()
{
    Lock<Mutex> lock(mutex_);
    trace_.reset();
}


////////////////////////////////////////////////////////////////
void ThreadImpl::notify_resuming()
{
    if (RefPtr<Target> target = runnable_.target())
    {
        target->debugger().on_resuming(*this);
    }
}


////////////////////////////////////////////////////////////////
void ThreadImpl::notify_queue()
{
    if (!is_stopped_by_debugger())
    {
        if (RefPtr<Target> t = runnable_.target())
        {
            if (t->is_attached(this))
            {
                t->debugger().queue_event(this);
            }
        }
    }
}

////////////////////////////////////////////////////////////////
void ThreadImpl::set_event_pending(bool flag)
{
    if (flag)
    {
        if (++eventPending_ > 1)
        {
            throw logic_error("too many pending events");
        }
    }
    else
    {
        assert(eventPending_ > 0);
        --eventPending_;
    }
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::is_event_pending() const
{
    return (eventPending_ > 0);
}


////////////////////////////////////////////////////////////////
size_t ThreadImpl::enum_cpu_regs(EnumCallback<Register*>* callback)
{
    size_t count = enum_user_regs(callback);
    return count + enum_fpxregs(callback);
}


////////////////////////////////////////////////////////////////
pid_t ThreadImpl::wait_internal(int* status, int opts, bool noThrow)
{
    pid_t pid = this->lwpid();

    if (noThrow)
    {
        pid = sys::waitpid(pid, status, opts, nothrow);
    }
    else
    {
        pid = sys::waitpid(pid, status, opts);
    }
    // assert(!status || (*status >> 16) == 0));
    assert(pid == lwpid() || (opts & WNOHANG));

    return pid;
}


////////////////////////////////////////////////////////////////
void ThreadImpl::wait_update_status(bool checkUnhandledThreads)
{
    int status = 0;

    RefPtr<UnhandledMap> umap;

    if (checkUnhandledThreads)
    {
        umap = interface_cast<UnhandledMap*>(debugger_);
        if (umap)
        {
            umap->query_status(lwpid(), &status, true);
        }
    }
    if (status)
    {
        dbgout(0) << "out of order event: " << hex << status << dec << endl;
    }
    else
    {
        wait_internal(&status, __WALL);
    }

    set_status(status);
}


////////////////////////////////////////////////////////////////
void ThreadImpl::detach()
{
    Lock<Mutex> lock(mutex_);
    trace_.reset();
    retVal_.reset();
    ptrace_wrapper(PTRACE_DETACH);
}



////////////////////////////////////////////////////////////////
void ThreadImpl::ptrace_wrapper(int req)
{
    if (lwpid() == 0)
    {
        cerr << "*** Warning: thread=" << thread_id();
        cerr << " not bound\n";
        return;
    }
    if (thread_finished(*this))
    {
        cerr << "*** Warning: " << __func__ << "(" << req << ") PID=";
        cerr << this->lwpid() << " not running?" << endl;
    }
    addr_t resume = 0;
    bool detach = false;

    switch (req)
    {
    case PTRACE_CONT:
    case PTRACE_SINGLESTEP:
    case PTRACE_SYSCALL:
        resume = 1;
        notify_resuming();
        clear_cached_regs();
        break;

    case PTRACE_DETACH:
    case PTRACE_KILL:
        detach = true;
        break;
    }

    //
    // Do not re-deliver SIGTRAP and SIGSTOP to debugged threads.
    //
    // The Linux ptrace man page says that PTRACE_CONT will not
    // re-deliver the SIGTRAP signal, but does not explicitly say
    // the same for PTRACE_SYSCALL and PTRACE_SINGLESTEP, so I
    // zero it out just to make sure.
    //
    if ((resume || detach) &&
        ((signal() == SIGTRAP) || (signal() == SIGSTOP)))
    {
        set_signal(0);
    }

    const int signum = this->signal();

    __ptrace_request ptrq = static_cast<__ptrace_request>(req);

    // resume == 1
    // BSD needs 1 as the resume address, Linux ignores it
    sys::ptrace(ptrq, this->lwpid(), resume, signum);
    if (resume)
    {
        resumed_ = true;
    }

    dbgout(1) << "ptrace(" << ptrq << ", " << lwpid()
              << ", signal=" << signum << ")=ok" << endl;

    if (!detach)
    {
        runnable_.refresh();
    }
}


////////////////////////////////////////////////////////////////
void ThreadImpl::step_instruction()
{
    // memorize the most recently seen signal
    const int lastSignal = this->signal();

    dbgout(2) << "single-stepping thread: " << lwpid() << endl;

    ptrace_wrapper(PTRACE_SINGLESTEP);

    // wait for the thread to receive a signal
    wait_update_status(true);

    if (this->signal() != SIGTRAP)
    {
        // don't know for sure if this can happen,
        // but just in case another signal was pending
        // and it got delivered before SIGTRAP: notify
        // the debugger to handle the event when control
        // returns to the DebuggerBase::run event loop

        notify_queue();
    }
    else if (WIFSTOPPED(status_))
    {
        if (lastSignal != SIGTRAP)
        {
            set_signal(lastSignal); // restore signal
        }
        check_thread_creation_events();
    }
}


////////////////////////////////////////////////////////////////
void ThreadImpl::check_thread_creation_events()
{
    // check for ptrace extension events
    const int event = (status_ >> 16);

    DEBUG_TRACE_EVENT(lwpid(), event, PTRACE_EVENT_FORK);

    if ((event == PTRACE_EVENT_FORK) || (event == PTRACE_EVENT_CLONE))
    {
        notify_queue();
    }
}


// This value is used when we want to compare the stack depth
// at a special breakpoint event with some initial value (when stepping
// over, returning from function calls) and helps determine if we are
// within a recursive function.

static size_t adjusted_stack_depth(Thread& thread)
{
    size_t depth = thread.stack_trace()->size();
    // hack: when the program first starts up
    // the trace unwinding may yield a short stack
    // i.e. miss a frame in the C startup code.
    if (depth <= 2)
    {
        depth = numeric_limits<size_t>::max();
    }
    else if (!StackTraceImpl::use_frame_handlers()
        || thread.stack_trace()->frame_handler_frame_count() + 1 < depth)
    {
        if (Frame* frame = thread_current_frame(&thread))
        {
            if (StackTraceImpl::check_prologue(thread, *frame))
            {
                ++depth;
            }
        }
    }
    return depth;
}


namespace
{
    /**
     * Helper breakpoint action for step_until_current_func_returns
     */
    class ZDK_LOCAL FuncRetAction : public ZObjectImpl<BreakPointAction>
    {
    public:
        FuncRetAction(Thread& thread, size_t depth) : depth_(depth)
        { }

        ~FuncRetAction() throw() { }

        const char* name() const { return "RETURN"; }

        word_t cookie() const { return depth_; }

        bool execute(Thread* thread, BreakPoint* bpnt)
        {
            assert(thread);

            // only break if at the same stack trace depth as
            // when the breakpoint was set, to deal with recursive
            // function calls
            const size_t stackDepth = thread->stack_trace()->size();

            if (stackDepth <= depth_)
            {
                ThreadImpl& threadImpl = interface_cast<ThreadImpl&>(*thread);
                threadImpl.check_return_value();

                if (threadImpl.is_stepping_to_return())
                {
                    threadImpl.set_stepping_to_return(false);
                }
                if (Debugger* dbg = thread->debugger())
                {
                    dbg->schedule_interactive_mode(thread, E_THREAD_RETURN);
                }
                return false; // discard action after execution
            }
            return true; // keep action
        }

    private:
        size_t depth_;
    };
}


////////////////////////////////////////////////////////////////
void ThreadImpl::step_until_current_func_returns()
{
    addr_t addr = 0;

    if (Frame* frame = thread_current_frame(this))
    {
        if (size_t i = frame->index())
        {
            frame = stack_trace()->frame(++i);
            if (frame)
            {
                addr = frame->program_count();
            }
        }
        else
        {
            addr = ret_addr();
        }
    }

    if (addr == 0)
    {
        // any better way to handle this? throw exception?
        cerr << "Cannot determine return address" << endl;
    }
    else if (addr != INVALID_PC_ADDR) // not in expression evaluation
    {
        if (Debugger* dbg = this->debugger())
        {
            size_t depth = adjusted_stack_depth(*this);
            if (depth)
            {
                --depth;
            }
            set_call_info(addr);

            stepReturn_ = true;
            actionContext_.reset(dbg->current_action_context());

            if (BreakPointManager* mgr = interface_cast<BreakPointManager*>(dbg))
            {
                RefPtr<BreakPointAction> act(new FuncRetAction(*this, depth));
                mgr->set_breakpoint(&runnable_, BreakPoint::PER_THREAD, addr, act.get());
            }

            dbgout(0) << ": context=" << actionContext_.get() << endl;
        }
    }
}



////////////////////////////////////////////////////////////////
void ThreadImpl::step_thru(addr_t first, addr_t last, ZObject* context)
{
    assert(first != last || (first == 0 && last == 0));
    assert(first);

    if (first)
    {
        stepRange_.reset(new StepRange(first, last, *this));
        actionContext_.reset(context);
    }
    else
    {
        stepRange_.reset();
        actionContext_.reset();
    }
}


namespace
{
    /**
     * A break point action do be execution upon reaching
     * the return address of the current function
     */
    class ZDK_LOCAL ActionStepOver : public ZObjectImpl<BreakPoint::Action>
    {
    public:
        explicit ActionStepOver(ThreadImpl::StepRange& step)
            : step_(step)
        {}

    private:
        const char* name() const { return "STEP_OVER"; }

        bool execute(Thread* thread, BreakPoint* bpnt)
        {
            step_.set_stepped_over(0);
            step_.set_in_stub(false);

            return false; // discard action after execution
        }

        word_t cookie() const { return 0; }

        ThreadImpl::StepRange& step_;
    };
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::set_step_over_breakpoint(addr_t addr)
{
    assert(debugger());

    BreakPointManager* mgr = CHKPTR(debugger()->breakpoint_manager());
    RefPtr<BreakPoint::Action> action = new ActionStepOver(*stepRange_);

    // arrange for an action to be invoked upon return
    if (!mgr->set_breakpoint(&runnable_, BreakPoint::HARDWARE, addr, action.get()))
    {
        dbgstep() << "Failed to set breakpoint at " << (void*)addr << endl;
        return false;
    }
    //... and memorize the addr of the stepped-over func
    stepRange_->set_stepped_over(addr);

    dbgstep() << "Temp breakpoint set " << (void*)addr << endl;
    return true;
}


////////////////////////////////////////////////////////////////
void ThreadImpl::single_step_now()
{
    set_single_step_mode(true, NULL);
    notify_resuming();

    ptrace_wrapper(PTRACE_SINGLESTEP);
    assert(resumed_);
}



////////////////////////////////////////////////////////////////
// A line of high-level code translates (most of the time) into
// a range of machine-level instructions. Stepping over a line
// of C/C++ code normally means stepping over a range of machine
// instructions.
//
// The debugger uses debug info (generated at compile-time) to
// map lines of high-level code to machine instruction ranges.
//
// This function executes when the main debugger event loop restarts
// the debuggee threads. If a step range is defined, we're going to
// go through it in single-step mode, unless some corner cases occur.
//
// See the comments inside this function for the corner cases.
//
bool ThreadImpl::resume_stepping()
{
    if (!stepRange_.get())
    {
        dbgout(2) << "no range" << endl;
        return false;
    }
    assert(stepRange_->end());

    const addr_t pc = ::program_count(*this);
    // The easiest case is that the current program counter is
    // within the range to be stepped through. Execute one machine
    // instruction (a single step) and we're done.
    if (is_addr_in_step_range(pc))
    {
        stepRange_->set_in_stub(false);
        single_step_now();

        return true;
    }
    else if (is_plt_jump(*this, pc))
    {
        // the PC may be out of range, but inside a Procedure Linkage Table
        // http://em386.blogspot.com/2006/10/resolving-elf-relocation-name-symbols.html
        dbgstep() << "is_plt_jump: " << (void*) pc << endl;

        stepRange_->set_in_stub(true);
        single_step_now();

        return true;
    }
    else
    {
        // The program counter is out of stepping range: we need to decide
        // if we're done stepping. If we are out of range because a function
        // call occurred, then single-stepping may be too slow; set an internal
        // breakpoint at the return address of that function, and continue at 
        // full speed until the internal breakpoint is hit (after that we may 
        // go back to single-stepping).

        dbgstep() << "range=[0x"
                  << hex << stepRange_->begin()
                  << ", 0x" << stepRange_->end()
                  << ") pc=0x" << pc
                  << " in_stub_code=" << stepRange_->in_stub()
                  << endl;

        // already stepped over a function call? then there should be an
        // internal breakpoint already set, resume execution at full speed
        if (stepRange_->stepped_over())
        {
            dbgstep() << "running until function returns [" << lwpid() << "]: "
                      << (void*)stepRange_->stepped_over() << endl;

            set_single_step_mode(false, NULL);

            // resume debuggee (which should break at ActionStepOver)
            return false;
        }

        // get the return address for the current function
        const addr_t addr = step_return_addr(*this);

        if ((addr > stepRange_->begin())    &&
            (addr <= stepRange_->end())     &&
            // equivalent, but faster than: is_addr_in_step_range(addr, true)
            (pc != stepRange_->end())       &&
            (!stepRange_->step_into() || stepRange_->skip_step(*this, pc))
           )
        {
            if (RefPtr<Symbol> s = symbols()->lookup_symbol(pc))
            {
                dbgstep() << s->name() << endl;

                // longjmp calls never return; keep stepping
                // while we're in the scope of "longjmp" (see below)
                // NOTE this hack is glibc/gcc specific and may not
                // work for other compilers. As of today (01/11/2012)
                // it works fine for clang++ compiled programs.
                if (strcmp(s->name()->c_str(), "_longjmp") == 0)
                {
                    stepRange_->set_longjump_called();
                    single_step_now();
                    return true;
                }
            }
            set_step_over_breakpoint(addr); // set internal breakpoint
        }
        else if (stepRange_->longjump_called())
        {
            // keep stepping until longjmp() returns
            if (RefPtr<Symbol> s = symbols()->lookup_symbol(pc))
            {
                if (strcmp(s->name()->c_str(), "longjmp") != 0)
                {
                    stepRange_->set_longjump_called(false);
                    set_single_step_mode(true, NULL);
                    return false;
                }
            }
            single_step_now();
            return true;
        }
        else
        {   // looks like we're really done stepping

            // is the debuggee about to return a value from a function?
            check_return_value();

            if ((stepRange_->end() < stepRange_->begin()) && (stepRange_->end() != pc))
            {
                dbgstep() << "reversed range" << endl;

                if (stepRange_->step_into() && stepRange_->ret_addr() == ret_addr())
                {
                    set_single_step_mode(true, NULL);
                    return false;
                }
            }

            // if we landed in the same function where we started, check that
            // the stack trace depth is the same as when the stepping range was
            // established (to avoid stopping at the wrong depth of a recursive call)

            if (stepped_recursive(pc))
            {
                return false;
            }
            if (pc != stepRange_->begin())
            {
                try
                {
                    assert (!is_plt_jump(*this, pc));
                    debugger()->set_temp_breakpoint(&runnable_, pc);
                }
                catch (const exception& e)
                {
                    cerr << __func__ << ": " << e.what() << endl;
                    set_single_step_mode(true, NULL);
                }
            }
            dbgstep() << lwpid() << ": done." << endl;
            stepRange_.reset();
            finishedStepping_ = true;
        }
    }
    return false;
}



////////////////////////////////////////////////////////////////
bool ThreadImpl::stepped_recursive(reg_t pc)
{
    if (!stepRange_->step_into()) // in stepping-over mode?
    {
        if (SymbolMap* syms = symbols())
        {
            // get the current function we ended up in
            if (RefPtr<Symbol> s = syms->lookup_symbol(pc))
            {
                // get the function where we started stepping
                RefPtr<Symbol> orig = syms->lookup_symbol(stepRange_->begin());

                // if we're in the same function where we started,
                // check for the stack depth to be the same -- otherwise
                // we may be in a recursive call
                if (orig && s->value() == orig->value())
                {
                    const size_t depth = stack_trace()->size();

                    // handle recursive function calls
                    if (depth > nextStackDepth_)
                    {
                        step_instruction();
                        set_step_over_breakpoint(pc);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_step_into(bool step)
{
    if (stepRange_.get())
    {
        stepRange_->set_step_into(step);
    }
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_call_info(addr_t caller)
{
    assert(stack_trace());

    if (Frame* frame = thread_current_frame(this))
    {
        called_ = frame->function();
        callerProgramCount_ = caller ? caller : ret_addr();
    }
}


////////////////////////////////////////////////////////////////
void ThreadImpl::check_return_value()
{
    Lock<Mutex> lock(mutex_);
    if (!called_)
    {
        return;
    }
    if (program_count() != callerProgramCount_)
    {
        return;
    }

    if (RefPtr<DebugSymbol> symbol = ret_symbol(this, called_))
    {
        RefPtr<DataType> returnType = symbol->type();
        assert(!interface_cast<FunType*>(symbol->type()));

        if (returnType && !returnType->name()->is_equal("void"))
        {
            retVal_ = symbol;
        }
        called_.reset();
        callerProgramCount_ = 0;
    }
}


////////////////////////////////////////////////////////////////
DebugSymbol* ThreadImpl::func_return_value()
{
    Lock<Mutex> lock(mutex_);
    return retVal_.get();
}


////////////////////////////////////////////////////////////////
void ThreadImpl::before_resume()
{
    Lock<Mutex> lock(mutex_);
    retVal_.reset();

    if (is_stopped_by_debugger())
    {
        assert(signal() == SIGSTOP);
        set_stopped_by_debugger(false);
    }
    if (finishedStepping_ && !stepRange_.get() && !stepReturn_)
    {
        actionContext_.reset();
    }
    finishedStepping_ = false;
    thread_set_event_description(*this, NULL);
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::resume(bool* stepped)
{
    if (has_resumed())
    {
        return true;
    }

    before_resume();

    if (resume_stepping())
    {
        assert(has_resumed());
        if (stepped)
        {
            *stepped |= true;
        }
        // have single-stepped the thread? done.
        dbgstep() << "stepped" << endl;
        return true;
    }

    if (RefPtr<Target> target = runnable_.target())
    {
        const uint64_t opts = target->debugger().options();
        int cmd = PTRACE_SYSCALL;

        if (single_step_mode())
        {
            cmd = PTRACE_SINGLESTEP;
        }
        else if ((opts & (Debugger::OPT_BREAK_ON_SYSCALLS
                        | Debugger::OPT_TRACE_SYSCALLS)) == 0)
        {
            if (target->has_linker_events())
            {
                cmd = PTRACE_CONT;
            }
        }

        ptrace_wrapper(cmd);

        assert(resumed_);
    }
    else
    {
        return false;
    }
    return true;
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::is_stopped_by_debugger() const
{
    bool result = false;

    if (debuggerStop_ && !thread_finished(*this))
    {
        result = (this->signal() == SIGSTOP);
    }
    return result;
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::is_done_stepping() const
{
    return finishedStepping_ || (singleStepMode_ && actionContext_);
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_user_data
(
    const char* key,
    word_t      val,
    bool        replace
)
{
    assert(key);
    ThreadBase::set_user_data(key, val, replace);
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::get_user_data(const char* key, word_t* val) const
{
    assert(key);
    return ThreadBase::get_user_data(key, val);
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_user_object
(
    const char* key,
    ZObject*    object,
    bool        replace
)
{
    assert(key);
    ThreadBase::set_user_object(key, object, replace);

    // hack: the expression interpreter has finished restoring
    // the registers after calling some function inside the
    // debugged program, the cached stack trace is stale by now
    // and needs to be reset; todo: add methods on Thread to
    // specifically deal with CPU state restoration
    if (!object && strcmp(key, ".regs") == 0)
    {
        reset_stack_trace();
    }
}


////////////////////////////////////////////////////////////////
ZObject* ThreadImpl::get_user_object(const char* key) const
{
    assert(key);
    return ThreadBase::get_user_object(key);
}


////////////////////////////////////////////////////////////////
bool
ThreadImpl::is_addr_in_step_range(addr_t addr, bool closedEnd) const
{
    bool result = false;

    if (stepRange_.get())
    {
        if (!is_syscall_pending(program_count()))
        {
            const addr_t end = stepRange_->end();

            result =
                (stepRange_->begin() <= addr) &&
                ((end > addr) || (closedEnd && end == addr));
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
/* bool ThreadImpl::is_stepped_over(addr_t addr) const
{
    if (stepRange_.get())
    {
        return addr == stepRange_->stepped_over();
    }
    return false;
}
*/


////////////////////////////////////////////////////////////////
void ThreadImpl::set_forked()
{
    forked_ = true;
    const DebugRegs::Condition cond = DebugRegs::BREAK_ON_ANY;

    const size_t count = CHKPTR(debugRegs_)->max_size(cond);
    // make sure the hardware debug registers are in a clean
    // state -- they might've been copied over from the parent
    for (size_t i = 0; i != count; ++i)
    {
        debugRegs_->enable(0, false, true,
                           cond,
                           DebugRegs::BREAK_ONE_BYTE // ignored
                          );
    }
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::is_line_stepping() const
{
    return stepRange_.get();
}


////////////////////////////////////////////////////////////////
pid_t ThreadImpl::get_signal_sender() const
{
    return sender_;
}


////////////////////////////////////////////////////////////////
DebugRegs* ThreadImpl::debug_regs()
{
    assert((debugRegs_.get() == NULL) || (&debugRegs_->thread() == this));

    return debugRegs_.get();
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_next(Symbol* sym)
{
    Lock<Mutex> lock(mutex_);
    // sym is the target, i.e. where we should stop stepping
    next_ = sym;
    // memorize the stack depth when we start to step over,
    // so that we deal correctly with recursive functions
    nextStackDepth_ = adjusted_stack_depth(*this);
}


////////////////////////////////////////////////////////////////
void ThreadImpl::set_stepping(bool stepping)
{
    isStepping_ = stepping;
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::is_stepping() const
{
    return isStepping_;
}


////////////////////////////////////////////////////////////////
bool ThreadImpl::exited(int* status) const
{
    if (status)
    {
        *status = status_;
    }
    return WIFEXITED(status_);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
