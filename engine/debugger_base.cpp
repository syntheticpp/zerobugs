//
// $Id: debugger_base.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2012 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include <assert.h>
#include <errno.h>          // errno, ECHILD
#ifdef HAVE_UNISTD_H
 #include <unistd.h>
#endif
#include <signal.h>         // NSIG
#include <algorithm>
#include <fstream>          // ifstream
#include <functional>
#include <iostream>
#include <sstream>          // ostringstream
#include "generic/lock.h"
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "zdk/fheap.h"
#include "zdk/shared_string_impl.h"
#include "dharma/canonical_path.h"
#include "dharma/config.h"
#include "dharma/directory.h"
#include "dharma/environ.h"
#include "dharma/exec.h"
#include "dharma/exec_arg.h"
#include "dharma/fstream.h"
#include "dharma/object_factory.h"
#include "dharma/pipe.h"
#include "dharma/process_name.h"
#ifdef __unix__
 #include "dharma/sigutil.h"
#endif
#include "dharma/settings.h"
#include "dharma/system_error.h"
#include "dharma/syscall_wrap.h"
#if HAVE_ELF
 #include "elfz/public/core_file.h"
 #include "elfz/public/error.h"
#endif
#include "target/target_factory.h"
#include "breakpoint_enabler.h"
#include "debugger_base.h"
#include "history.h"
#include "ptrace.h"
#include "module.h"
#include "signal_policy.h"
#include "thread.h"


#ifndef NSIG
 #define NSIG _NSIG
#endif

using namespace std;
using namespace eventlog;


static const word_t defaultOpts = Debugger::OPT_HARDWARE_BREAKPOINTS |
                                  Debugger::OPT_START_AT_MAIN        |
                                  Debugger::OPT_TRACE_FORK;

////////////////////////////////////////////////////////////////
TargetFactory& DebuggerBase::target_factory()
{
    return TheTargetFactory::instance();
}


////////////////////////////////////////////////////////////////
DebuggerBase::DebuggerBase()
    : verbose_(0)
    , haveNewProgram_(false)
    , quit_(false)
    , signaled_(false)
    , initialThreadFork_(false)
    , historySnapshotsEnabled_(true)
    , options_(defaultOpts)
    , lwpidStep_(0)
    , unhandled_(new UnhandledMap)
    , factory_(new ObjectFactoryImpl(this))
    , startupTime_(time(NULL))
{
}


////////////////////////////////////////////////////////////////
DebuggerBase::~DebuggerBase() throw()
{
    detach(nothrow);
}


////////////////////////////////////////////////////////////////
bool DebuggerBase::is_attached() const
{
    Lock<Mutex> lock(TargetManager::mutex());

    const TargetManager::const_iterator e = end(lock);
    for (TargetManager::const_iterator i = begin(lock); i != e; ++i)
    {
        if ((*i)->is_attached())
        {
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
void DebuggerBase::set_environment(const char* const* begin)
{
    assert(begin);

    if (begin != env_.cstrings())
    {
        const char* const* end = begin;
        while (*end)
        {
            ++end;
        }
        env_.clear();
        env_.assign(begin, end);
    }
}


////////////////////////////////////////////////////////////////
const char* const* DebuggerBase::environment(bool reset)
{
    if (reset || env_.strings().empty())
    {
        // make a copy of the current (runtime) environment
        extern char** environ;
        const_cast<DebuggerBase*>(this)->set_environment(environ);
    }
    return env_.cstrings();
}


////////////////////////////////////////////////////////////////
pid_t DebuggerBase::exec(
    const char* cmd,
    bool shellExpandArgs,
    const char* const* env)
{
    dbgout(0) << __func__ << ": " << cmd << endl;

    ExecArg args(cmd);
    return exec(args, shellExpandArgs, env);
}



////////////////////////////////////////////////////////////////
static void shell_expand_args(ExecArg& args)
{
#ifdef __unix__
    string shell;
    if (const char* p = getenv("SHELL"))
    {
        shell = p;
    }
    else
    {
        shell = "/bin/sh";
    }

    Pipe pipe;

    deque<string> argv;
    argv.push_back(shell);
    argv.push_back("-c");
    argv.push_back("echo " + args.command_line());

    // mimic system() behavior: block SIGCHLD
    // and ignore SIGINT and SIGQUIT
    BlockSignalsInScope block(SIGCHLD);
    IgnoreSignalInScope ignore(SIGINT);
    IgnoreSignalInScope ignore2(SIGQUIT);

    int pid = ::exec(shell, argv, pipe.input());

    char buf[512]; // for reading the output
    string result; // the expanded command
    for (;;)
    {
        ssize_t rc = read(pipe.output(), buf, sizeof buf - 1);
        if (rc < 0)
        {
            if (errno == EAGAIN)
            {
                continue;
            }
            ::waitpid(pid, 0, 0);
            throw SystemError(__func__ + string(": read"));
        }
        if (rc)
        {
            buf[rc] = 0;
            result += buf;
        }
        if (static_cast<size_t>(rc) < sizeof buf - 1)
        {
            int waitRes = ::waitpid(pid, 0, WNOHANG);
            if (waitRes == pid)
            {
                break; // shell is done
            }
            if (waitRes < 0)
            {
                throw SystemError(__func__ + string(": waitpid"));
            }
        }
    }
    // trim trailing spaces, '\n', etc.
    while (size_t n = result.size())
    {
        if (isspace(result[--n]))
        {
            result.resize(n);
        }
        else
        {
            break;
        }
    }
#if DEBUG
    clog << __func__ << ": " << result << endl;
#endif
    ExecArg(result).swap(args);
#endif // __unix__
}


////////////////////////////////////////////////////////////////
static bool check_for_interpreter(ExecArg& arg)
{
    bool result = false;
    ifstream fs(arg[0]);
    vector<char> buf(1024);

    if (fs.getline(&buf[0], buf.size() - 1))
    {
        // starts with #! magic sequence?
        if (buf.size() >= 2 && buf[0] == '#' && buf[1] == '!')
        {
            istringstream is(&buf[2]);

            string s;
            while (is >> s)
            {
                arg.push_front(s);
                result = true;
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
pid_t DebuggerBase::exec (
    const ExecArg& execArgs,
    bool shellExpandArgs,
    const char* const* env)
{
    ExecArg args(execArgs);

    string protocol;

    if (const char* p = strstr(args[0], "://"))
    {
        protocol.assign(args[0], distance(args[0], p));
        dbgout(0) << __func__<< ": " << protocol << endl;
    }

    // Canonicalize and expand arguments in shell, if needed;
    // this only works for local targets.
    if (protocol.empty())
    {
        // canonicalize
        if (!args.strings().empty())
        {
            string path = CanonicalPath(args.strings().front());
            args.pop_front();
            args.push_front(path);
        }
        if (shellExpandArgs)
        {
            shell_expand_args(args);
        }
        if (options_ & OPT_TRACE_FORK)
        {
            check_for_interpreter(args);
        }
    }
    // make a copy, if env points into the history or into
    // another Process' environment it will go away when
    // detach() is called below
    SArray localEnviron(env);
    if (is_attached())
    {
        detach();
    }
    if (!env)
    {
        env = this->environment();
    }
    else
    {
        env = localEnviron.get();
    }
    assert(TargetManager::empty());

    //the first arg may contain target params, pass it on
    TargetPtr target = Target::new_live_target(*this, args[0]);
    assert(target); // new_live_target() should never return NULL

    pid_t pid = 0;

    if (RefPtr<Thread> t = target->exec(args, env))
    {
        assert(target->process()); // post-condition
    #ifdef __linux__
        assert(target->process()->pid() == t->lwpid());
    #endif
        pid = CHKPTR(target->process())->pid();
        target->process()->set_environment(env);

        set_have_new_program(true);
        queue_event(interface_cast<ThreadImpl>(t));
    }
    return pid;
}



////////////////////////////////////////////////////////////////
bool DebuggerBase::try_attach(string& arg, size_t argc, const char* filename)
{
    if (strncmp("--pid=", arg.c_str(), 6))
    {
        return false;
    }
    else if (pid_t pid = strtol(arg.c_str() + 6, 0, 10))
    {
        arg.clear();
        attach(pid);

        if (Thread* thread = get_thread(DEFAULT_THREAD))
        {
            if (filename)
            {
                const string path = canonical_path(filename);
                if (strcmp(path.c_str(), thread->filename()) != 0)
                {
                    throw runtime_error("process does not match path: " + path);
                }
            }
            if (argc > 1)
            {
                cout << "*** Warning: arguments ignored when ";
                cout << "attaching to a running process\n";
            }
            on_event(*thread);
            return true;
        }
    }
    else
    {
        throw invalid_argument("cannot attach to pid 0");
    }
    return false;
}


////////////////////////////////////////////////////////////////
static bool is_corefile(const char* filename)
{
    assert(filename);

#if HAVE_ELF
    int elf_type = 0;
    try
    {
        ELF::Binary bin(filename);
        elf_type = bin.header().type();
    }
    catch (const ELF::Error&)
    {
        return false;
    }
    catch (const SystemError& e)
    {
        if (e.error() != ENOENT)
        {
            throw;
        }
    }
    return elf_type == ET_CORE;
#else
	return false;
#endif
}


////////////////////////////////////////////////////////////////
void DebuggerBase::attach_or_exec(const ExecArg& args)
{
    if (args.empty())
    {
        return;
    }
    string program = args.strings()[0];
    string error;
    try
    {
        if (try_attach(program, args.size()))
        {
            // attach succeeded, nothing else to do
            dbgout(0) << __func__ << ": attached to " << program << endl;
        }
        else if (!program.empty())
        {
            const char* filename = program.c_str();

            if (is_corefile(filename))
            {
                if (args.size() > 1)
                {
                    // Assume that the command line argument following
                    // the core file name is a program file name.
                    load_core(filename, args.strings()[1].c_str());
                }
                else
                {
                    // The name of the program file that caused the
                    // core dump attempt will be determined from
                    // information in the core file (if available).
                    load_core(filename, NULL);
                }
                if (args.size() > 2)
                {
                    cout << "*** Warning: extra arguments ignored\n";
                }
            }
            else
            {
                // for compatibility with GDB, allow the following
                // command lines to work:
                // zero <program> <PID>
                // zero <program> <corefile>
                if (args.size() > 1)
                {
                    const deque<string>& argv = args.strings();
                    const size_t n = args.size() - 1;
                    string pid = argv[1];
                    if (try_attach(pid, n, program.c_str()))
                    {
                        return; // attached successfully, done
                    }
                    if (is_corefile(argv[1].c_str()))
                    {
                        load_core(argv[1].c_str(), argv[0].c_str());
                        if (args.size() > 2)
                        {
                            cout << "*** Warning: extra args ignored\n";
                        }
                        return;
                    }
                }
                dbgout(0) << __func__ << ": exec-ing " << program << endl;
                exec(args, false, NULL);
            }
        }
    }
    catch (const exception& e)
    {
        error = e.what();
    #if DEBUG
        cerr << __func__ << ": " << error << endl;
    #endif
    }
    catch (...)
    {
        error = "Unknown Error";
    }
    if (!error.empty())
    {
        if (!program.empty())
        {
            error = program + ": " + error;
        }
        cerr << error << endl;
        critical_error(NULL, error.c_str());
        detach_all_targets();
    }
}


////////////////////////////////////////////////////////////////
bool DebuggerBase::has_corefile() const
{
    Lock<Mutex> lock(TargetManager::mutex());
    if (!TargetManager::empty(lock))
    {
        RefPtr<Target> target = *TargetManager::begin(lock);
        if (RefPtr<Process> proc = target->process())
        {
            return proc->origin() == ORIGIN_CORE;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////
void DebuggerBase::load_core(const char* corefile, const char* progfile)
{
    assert(corefile);
    if (is_attached())
    {
        detach();
    }

    assert(TargetManager::empty());
    TargetPtr target = Target::new_core_target(*this);

    ExecArg args;
    args.push_back(corefile ? corefile : "");
    args.push_back(progfile ? progfile : "");
    target->exec(args, NULL);

    add_target(target);

    assert(!TargetManager::empty());
    assert(target->ref_count() > 1);
}


////////////////////////////////////////////////////////////////
size_t DebuggerBase::enum_user_tasks (
    EnumCallback<const Runnable*>* callback,
    const char* targetParam)
{
    unsigned int count = 0;
    TargetPtr target = Target::new_live_target(*this, targetParam);
    const string procfs = target->procfs_root();
    dbgout(0) << __func__ << ": " << procfs << endl;

    const Directory procDir(procfs, "[0-9]*");

    Directory::const_iterator i(procDir.begin());
    for (; i != procDir.end(); ++i)
    {
        const string tmp = (*i).substr(procfs.size() + 1);
        const pid_t pid = strtol(tmp.c_str(), 0, 0);

        if (XTrace::kill(pid, 0) < 0) // have enough rights?
        {
            continue;
        }
        ++count;

        if (callback)
        {
            try
            {
                RunnableImpl task(pid, target.get());

                // read info from the /proc filesystem
                task.refresh();

                callback->notify(&task);
            }
            catch (exception& e)
            {
                cerr << __func__ << ": "<< pid;
                cerr << ": " << e.what() << endl;
            }
        }
    }
    return count;
}


////////////////////////////////////////////////////////////////
void DebuggerBase::attach(pid_t pid, const char* targetParam)
{
    if (is_attached())
    {
        detach();
    }
    assert(TargetManager::empty());

    TargetPtr target = Target::new_live_target(*this, targetParam);
    target->attach(pid);

    // post-conditions:
    assert(!TargetManager::empty());
    assert(target->ref_count() > 1);
}



////////////////////////////////////////////////////////////////
void DebuggerBase::enable_breakpoints(bool onOff, int mask)
{
    BreakPointEnabler callback(onOff, mask);
    breakpoint_manager()->enum_breakpoints(&callback);
}


////////////////////////////////////////////////////////////////
void DebuggerBase::detach_all_targets()
{
    Lock<Mutex> lock(TargetManager::mutex());

    while (!TargetManager::empty(lock))
    {
        TargetManager::iterator i = begin(lock);
        RefPtr<Target> target = *i;

        if (Process* proc = target->process())
        {
            const pid_t pid = proc->pid();
            target->detach();
            cout << "Detached from process " << pid << "\n";
        }

        remove_target(*i);
    }
    assert(TargetManager::empty(lock));

    // empty the queue one more time -- just in case
    // any events were queued whilse stopping the threads
    queue_ = ThreadQueue();

    unhandled_.reset(new UnhandledMap);
}


////////////////////////////////////////////////////////////////
void DebuggerBase::detach()
{
    save_properties();

    queue_ = ThreadQueue(); // empty the pending events queue
    detach_all_targets();
    assert(TargetManager::empty());

    resume();
}


////////////////////////////////////////////////////////////////
bool DebuggerBase::detach(const nothrow_t&)
{
    bool result = false;
    try
    {
        detach();
        result = true;
    }
#ifdef DEBUG
    catch (const exception& e)
    {
        cerr << "DebuggerBase::detach: " << e.what() << endl;
    }
#endif
    catch (...)
    {
    }
    return result;
}


////////////////////////////////////////////////////////////////
void DebuggerBase::run()
{
    while (!quit_)
    {
        if (TargetManager::empty())
        {
            on_idle();
        }
        else
        {
            string error;

            RefPtr<Thread> tptr;

            if (has_corefile())
            {
                tptr = get_thread(DEFAULT_THREAD);
            }
            else try
            {
                tptr = get_event();
                if (!tptr)
                {
                    continue;
                }
            }
            catch (const exception& e)
            {
                error = e.what();
            }

            if (!error.empty())
            {
                cerr << "***** " << __func__ << ": " << error << " *****" << endl;

                critical_error(tptr.get(), error.c_str());

                error.clear();
                tptr.reset();

                // attempt to re-stabilize
                if (detach(nothrow))
                {
                    continue;
                }
                return;
            }

            if (RefPtr<Target> target = tptr->target())
            {
                if (tptr->is_stopped_by_debugger())
                {
                    dbgout(1) << tptr->lwpid() << ": stopped by debugger" << endl;
                }
                else
                {
                    try
                    {
                        target->handle_event(tptr.get());
                    }
                    catch (const exception& e)
                    {
                        error = e.what();
                    }

                    if (!error.empty())
                    {
                        error += ". Ok to continue?";
                        if (!message(error.c_str(), MSG_YESNO, tptr.get()))
                        {
                            detach(nothrow);
                        }
                        error.clear();
                    }
                }
            }
            else
            {
                dbgout(0) << tptr->lwpid() << ": NULL target" << endl;
            }
        }

        // resume all threads if no more events pending
        if (queue_.empty() && !has_corefile())
        {
            resume_threads();
        }
    }
    detach();
}


#if defined(DEBUG_OBJECT_LEAKS)
////////////////////////////////////////////////////////////////
void DebuggerBase::print_counted_objects(const char* func) const
{
    ObjectManager::instance().print(cout, func) << endl;
}
#else
void DebuggerBase::print_counted_objects(const char*) const
{
}
#endif


////////////////////////////////////////////////////////////////
void DebuggerBase::quit()
{
    if (quit_)
    {
        cerr << "*** Warning: already quitting\n";
    }
    Fheap<16>::drop();

    quit_ = true;
}


////////////////////////////////////////////////////////////////
void DebuggerBase::on_attach(Thread& thread)
{
    //clog << "attached: " << thread.name() << "\n";
    thread_set_event_description(thread, "Attached");
}


////////////////////////////////////////////////////////////////
void DebuggerBase::on_update(Thread& thread)
{
    if (thread_finished(thread))
    {
        dbgout(0) << __func__ << ": invoking cleanup" << endl;
        cleanup(thread);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerBase::cleanup(Thread& thread)
{
    const pid_t pid = CHKPTR(thread.process())->pid();

    dbgout(0) << __func__ << ": " << thread.lwpid()
              << " status=" << thread.status() << endl;

    RefPtr<Target> target = find_target(pid);
    if (!target)
    {
        throw logic_error(__func__ + string(": target not found"));
    }
    if (target->cleanup(thread) == 0)
    {
        dbgout(0) << __func__ << ": last thread in target" << endl;
        assert(target->enum_threads() == 0);

        take_snapshot();
        // if the number of remaining threads after cleanup
        // is zero, then remove this target
        remove_target(target);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerBase::on_resuming(Thread& thread)
{
#ifdef DEBUG
    // old linux threads stuff
    if (is_pthread_signal(thread.signal()))
    {
        clog << "thread " << thread.lwpid() ;
        clog << " got signal=" << thread.signal() << endl;
    }
#endif
    if (thread_stopped(thread) &&
       !signal_policy(thread.signal())->pass())
    {
        // don't pass the signal to the thread
        thread.set_signal(0);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerBase::on_resumed()
{
}


////////////////////////////////////////////////////////////////
void DebuggerBase::on_idle()
{
    dbgout(0) << " no more threads." << endl;
    exit(0);
}


////////////////////////////////////////////////////////////////
void DebuggerBase::on_event(Thread& thread)
{
    if (!is_silent())
    {
        print_event_info(cout, thread);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerBase::print_event_info(ostream& out, const Thread& thread)
{
#ifdef __unix__
    out << endl << "[" << thread.lwpid() << "]: ";
    const int status = thread.status();

    if (WIFSTOPPED(status))
    {
        out << "status=0x" << hex << status << dec << ' ';
        if (int signum = thread.signal())
        {
            out << sig_description(signum);
        }
        out << endl;
    }
    else
    {
        if (WIFEXITED(status)) // exited normally?
        {
            out << "exit code=" << WEXITSTATUS(status);
        }
        else
        {
            // or was it killed by a signal?
            assert(WIFSIGNALED(status));
            out << sig_description(thread.signal());
        }
        out << endl;
    }
    if (SharedString* desc = thread_get_event_description(thread))
    {
        out << desc->c_str() << endl;
    }
#endif
}


////////////////////////////////////////////////////////////////
RefPtr<ThreadImpl> DebuggerBase::peek_event(bool remove)
{
    RefPtr<ThreadImpl> result;

    while (!result && !queue_.empty())
    {
        result = queue_.front();

        dbgout(0) << __func__ << ": " << result->lwpid() << endl;

        if (remove)
        {
            queue_.pop();
        }

        result->set_event_pending(false);

        if (verbose())
        {
            clog << "########## " << " " << result->lwpid() << ": ";
            clog << hex << result->status() << dec << " ##########\n";
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
RefPtr<Thread> DebuggerBase::get_event()
{
#ifdef __unix__
    // make sure that we're not interrupted by SIGCHLD signals
    BlockSignalsInScope block(SIGCHLD);
    RefPtr<ThreadImpl> tptr = peek_event(true);

    while (!tptr)
    {
        int status = 0;
        dbgout(1) << __func__ << " calling waitpid" << endl;

        // peek_event() returned null, no event queued;
        // block until a debugged thread gets signaled
        pid_t pid = XTrace::waitpid(-1, &status, __WALL | WUNTRACED);

        if (pid < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            break;
        }
        assert(pid > 0);

        dbgout(1) << "*************** " << pid << ": " << hex << status << dec << endl;

        RefPtr<Target> target = find_target(pid);
        if (!target)
        {
            save_lwpid_and_status(pid, status);
            dbgout(0) << "Target not found for: " << pid << endl;
            continue;
        }

        if (Thread* thread = target->event_pid_to_thread(pid))
        {
            tptr = &interface_cast<ThreadImpl&>(*thread);
            tptr->set_status(status);

            if (!thread_is_attached(*tptr))
            {
                dbgout(0) << __func__ << ": thread detached" << endl;
                tptr.reset();
                if (TargetManager::empty())
                {
                    break;
                }
                continue;
            }
            if (target->event_requires_stop(thread))
            {
                stop_all_threads(thread);
            }
            else
            {
                tptr->resume();
                tptr.reset();
            }
        }
    }
    return tptr;
#else
    //
    // todo
    //
    return NULL;
#endif
}


////////////////////////////////////////////////////////////////
void DebuggerBase::stop_all_threads(Thread* skipThread)
{
    Lock<Mutex> lock(TargetManager::mutex());

    TargetManager::iterator end = this->end(lock);
    for (TargetManager::iterator i = begin(lock); i != end; ++i)
    {
        (*i)->stop_all_threads(skipThread);
    }
}


////////////////////////////////////////////////////////////////
void DebuggerBase::queue_event(const RefPtr<ThreadImpl>& thread)
{
    int status = 0;
	
    if (thread->exited(&status))
    {
        dbgout(0) << __func__ << ": exited: " << thread->lwpid() << endl;
        return;
    }

    DEBUG_TRACE_EVENT(thread->lwpid(), status, PTRACE_EVENT_FORK);
    assert(!thread->is_event_pending());

#ifdef __unix__
    if (!signal_policy(thread->signal())->stop())
    {
        return;
    }
#endif
    thread->set_event_pending(true);
    queue_.push(thread);

#ifdef __unix__
    dbgout(0) << "event queued: " << thread->lwpid()
              << ": " << sig_description(thread->signal())
           // dangerous side-effect: ptrace(PTRACE_GETREGS,...)
           //   may fail, syscall_wrappers.cpp throws exception
           // << " pc=" << (void*)thread->program_count();
              << ", signal sent by: " << thread->get_signal_sender()
              << endl;

    assert((thread->signal() != SIGSTOP) || !thread->is_stop_expected());
#endif // __unix__
}


////////////////////////////////////////////////////////////////
void DebuggerBase::set_default_signal_policies()
{
    assert(signalPolicies_.empty());
    assert(signalPolicies_.capacity() == 0);

    signalPolicies_.reserve(NSIG);

    for (int i = 0; i != NSIG; ++i)
    {
        SignalPolicyPtr pol(new SignalPolicyImpl(*this, i));
        signalPolicies_.push_back(pol);
    }
}


////////////////////////////////////////////////////////////////
SignalPolicy* DebuggerBase::signal_policy(int signum)
{
    if (signum < 0 || (size_t)signum >= signalPolicies_.size())
    {
        throw_signal_out_of_range(__func__, signum);
    }
    return signalPolicies_[signum].get();
}


////////////////////////////////////////////////////////////////
void DebuggerBase::resume_threads()
{
    size_t resumedCount = 0;
    bool empty = true;

    Lock<Mutex> lock(TargetManager::mutex());

    TargetManager::iterator i = TargetManager::begin(lock);
    const TargetManager::iterator targetsEnd = TargetManager::end(lock);

    for (; i != targetsEnd; ++i)
    {
        if ((*i)->enum_threads())
        {
            empty = false;

            resumedCount += (*i)->resume_all_threads();
        }
    }
    if (resumedCount)
    {
        on_resumed();
    }
    else if (empty)
    {
        dbgout(0) << __func__ << ": detaching" <<endl;
        detach();
    }
}


////////////////////////////////////////////////////////////////
void DebuggerBase::stop()
{
    Lock<Mutex> lock(TargetManager::mutex());

    const TargetManager::iterator end = TargetManager::end(lock);
    TargetManager::iterator i = TargetManager::begin(lock);
    for (; i != end; ++i)
    {
        (*i)->stop_async();
        break; // just stop the first target, the others
               // will be stopped synchronously later, when
               // the SIGSTOP event comes in
    }
}


////////////////////////////////////////////////////////////////
SymbolTableEvents* DebuggerBase::symbol_table_events()
{
    return 0;
}



////////////////////////////////////////////////////////////////
Thread* DebuggerBase::get_thread(pid_t lwpid, unsigned long tid) const
{
    Thread* result = NULL;
    Lock<Mutex> lock(TargetManager::mutex());
    const TargetManager::const_iterator e = end(lock);

    for (TargetManager::const_iterator i = begin(lock); i != e; ++i)
    {
        result = (*i)->get_thread(lwpid, tid);
        if (result)
        {
            break;
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t DebuggerBase::enum_threads(EnumCallback<Thread*>* cb)
{
    size_t result = 0;
    Lock<Mutex> lock(TargetManager::mutex());

    const TargetManager::iterator end = TargetManager::end(lock);
    TargetManager::iterator i = TargetManager::begin(lock);
    for (; i != end; ++i)
    {
        result += (*i)->enum_threads(cb);
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t DebuggerBase::enum_processes(EnumCallback<Process*>* cb) const
{
    size_t result = 0;
    Lock<Mutex> lock(TargetManager::mutex());

    const TargetManager::const_iterator end = this->end(lock);
    TargetManager::const_iterator i = this->begin(lock);
    for (; i != end; ++i)
    {
        if (Process* process = (*i)->process())
        {
            ++result;
            if (cb)
            {
                cb->notify(process);
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
string DebuggerBase::get_config_path()
{
    string cfgPath;

    const char* path = getenv("ZERO_CONFIG_PATH");
    if (!path)
    {
        path = getenv("HOME");
    }

    if (path)
    {
        cfgPath = path;
    }

    if (!cfgPath.empty() && cfgPath[cfgPath.size() - 1] != '/')
    {
        cfgPath += '/';
    }

    if (cfgPath.empty())
    {
        // store configuration in the parent dir of bin/zero
        cfgPath = realpath_process_name();
        size_t n = cfgPath.rfind("/bin/");
        if (n == string::npos)
        {
            n = cfgPath.rfind('/');
        }
        cfgPath.erase(++n);
        setenv("ZERO_CONFIG_PATH", cfgPath.c_str(), 1);
    }
    cfgPath += ".zero/";
    sys::mkdir(cfgPath, 0750);

#if DEBUG
    clog << __func__ << ": " << cfgPath << endl;
#endif
    return cfgPath;
}


////////////////////////////////////////////////////////////////
static void set_asm_syntax(Properties& properties)
{
    const string syntax = env::get_string("ZERO_ASM_SYNTAX", "");

    if (syntax.empty()) // not overriden by environment
    {
        switch (properties.get_word("asm_syntax", 0))
        {
        case Disassembler::ASM_ATT:
            setenv("ZERO_ASM_SYNTAX", "att", 1);
            break;

        case Disassembler::ASM_INTEL:
            setenv("ZERO_ASM_SYNTAX", "intel", 1);
            break;
        }
    }
}


static void copy_option(word_t w, uint64_t& options, Debugger::Option flag)
{
    if (w & flag)
    {
        options |= flag;
    }
    else
    {
        options &= ~flag;
    }
}


////////////////////////////////////////////////////////////////
//
// Initialize the settings object (if needed) and read the
// persisted state from the config path (including various
// options, and the state of previously debugged program --
// aka History)
//
Properties* DebuggerBase::properties()
{
    Lock<Mutex> lock(settingsMutex_);
    if (!settings_)
    {
        CHKPTR(factory_)->register_interface(History::_uuid(), HistoryImpl::create);
        settings_ = new Settings(factory_);

        string path = get_config_path();
        settings_->set_string("config_path", path.c_str());

        if (settingsPath_.empty())
        {
            settingsPath_ = path + "config";
        }
        FileStream fs(settingsPath_.c_str());

        size_t bytesRead = 0;
        while ((bytesRead = fs.read(settings_.get())) != 0)
        {
            dbgout(2) << bytesRead << " byte(s) read" << endl;
        }
        if (!CHKPTR(settings_)->get_object("hist"))
        {
            RefPtr<History> history = new HistoryImpl(factory_, path);
            settings_->set_object("hist", history.get());
        }
        const word_t opts = settings_->get_word("opts", defaultOpts);
        copy_option(opts, options_, OPT_BREAK_ON_THROW);

        // environment takes over saved value, apply saved
        // option only if environment var not set
        if (env::get("ZERO_START_MAIN", -1) == -1)
        {
            copy_option(opts, options_, OPT_START_AT_MAIN);
        }
        set_asm_syntax(*settings_);

        copy_option(opts, options_, OPT_TRACE_FORK);
        copy_option(opts, options_, OPT_SPAWN_ON_FORK);
    }
    return settings_.get();
}


////////////////////////////////////////////////////////////////
void DebuggerBase::refresh_properties()
{
    RefPtr<Settings> settings = settings_;
    if (settings)
    {
        settings->erase_opaque_objects();
    }
    settings_.reset();
    properties(); // re-initialize

    if (settings && settings_)
    {
        settings->merge(*settings_);
    }
    settings_ = settings;
}


////////////////////////////////////////////////////////////////
void DebuggerBase::take_snapshot()
{
    if (historySnapshotsEnabled_ && settings_)
    {
        RefPtr<HistoryImpl> history =
            interface_cast<HistoryImpl*>(settings_->get_object("hist"));

        if (history)
        {
            history->take_snapshot(*this);
        }
    }
}


////////////////////////////////////////////////////////////////
const HistoryEntry* DebuggerBase::get_most_recent_history_entry()
{
    const HistoryEntry* entry = NULL;

    if (const Properties* prop = properties())
    {
        if (RefPtr<HistoryImpl> history =
            interface_cast<HistoryImpl*>(prop->get_object("hist")))
        {
            if (history->entry_count())
            {
                entry = history->entry(0);
            }
        }
    }
    else
    {
        dbgout(0) << __func__ << ": null settings object\n";
    }
    return entry;
}



////////////////////////////////////////////////////////////////
RefPtr<HistoryEntryImpl>
DebuggerBase::get_history_entry(Process& proc) const
{
    RefPtr<HistoryEntryImpl> entry;

    if (settings_)
    {
        if (RefPtr<HistoryImpl> history =
            interface_cast<HistoryImpl*>(settings_->get_object("hist")))
        {
            if (const char* name = proc.name())
            {
                string path(name);
                map_path(&proc, path);
                // is there an entry for the current program?
                entry = history->get_entry_impl(path.c_str());

                if (RefPtr<WatchList> w = interface_cast<WatchList>(entry))
                {
                    interface_cast<ProcessImpl*>(&proc)->set_watches(w);
                }
            }
        }
    }
    return entry;
}


namespace
{
    /**
     * Helper callback for restoring breakpoints in the
     * debugged program.
     *
     * @see DebuggerBase::read_process_history
     * @see DebuggerEngine::on_attach
     */
    class ZDK_LOCAL RestorerCallback : public EnumCallback<Module*>
    {
        DebuggerBase& debugger_;
        Process& process_;
        RefPtr<HistoryEntryImpl> histEnt_;

        void notify(Module* mod)
        {
            if (mod)
            {
                debugger_.restore_module(process_, *mod, histEnt_);
            }
        }

    public:
        RestorerCallback
          (
            DebuggerBase& debugger,
            Process& process,
            RefPtr<HistoryEntryImpl> histEnt
          )
          : debugger_(debugger)
          , process_(process)
          , histEnt_(histEnt)
        { }
    };
}


////////////////////////////////////////////////////////////////
void DebuggerBase::read_process_history(Process& process)
{
    if (RefPtr<HistoryEntryImpl> entry = get_history_entry(process))
    {
        if (RefPtr<SymbolMap> symbols = process.symbols())
        {
            // remember all modules that have previously seen in this app
            HistoryEntryImpl::const_iterator i = entry->modules_begin();
            for (; i != entry->modules_end(); ++i)
            {
                const RefPtr<ModuleImpl>& mod = i->second;
                try
                {
                    symbols->add_module(mod->name()->c_str());
                }
                catch (const exception& e)
                {
                    clog << e.what() << endl;
                }
            }
        }

        RestorerCallback restorer(*this, process, entry);
        process.enum_modules(&restorer);
    }
}


////////////////////////////////////////////////////////////////
bool DebuggerBase::restore_module(
    Process& proc,
    Module& currentMod,
    RefPtr<HistoryEntryImpl> entry)
{
    // disable history snapshot while restoring breakpoints,
    // because setting a breakpoint invalidates the current
    // history snapshot
    Temporary<bool> setFlag(historySnapshotsEnabled_, false);
    bool result = false;

    if (!entry)
    {
        entry = get_history_entry(proc);
    }
    if (entry)
    {
        // use the module's image saved in the history
        // as a template for restoring breakpoints in the
        // current module

        if (RefPtr<ModuleImpl> module =
             entry->get_module(currentMod.name())
           )
        {
            module->restore(*this, proc, currentMod);
            result = true;
        }
    }

    return result;
}


namespace
{
    /**
     * Helper callback functor user by DebuggerBase::enum_modules
     */
    class ZDK_LOCAL EnumModuleHelper : public EnumCallback<Process*>
    {
    public:
        explicit EnumModuleHelper(EnumCallback<Module*>* cb)
            : callback_(cb), count_(0) {}

        /// EnumCallback<Process*> interface
        void notify(Process* process)
        {
            if (process)
            {
                count_ += process->enum_modules(callback_);
            }
        }

        /// @return number of total modules
        size_t count() const { return count_; }

    private:
        EnumCallback<Module*>* callback_;
        size_t count_;
    };
}// namespace


////////////////////////////////////////////////////////////////
size_t DebuggerBase::enum_modules(EnumCallback<Module*>* callback) const
{
    EnumModuleHelper helper(callback);

    enum_processes(&helper);
    return helper.count();
}


////////////////////////////////////////////////////////////////
void DebuggerBase::clear_properties()
{
    Lock<Mutex> lock(settingsMutex_);
    settings_.reset();
}


////////////////////////////////////////////////////////////////
void DebuggerBase::reset_properties()
{
    Lock<Mutex> lock(settingsMutex_);

    if (unlink(settingsPath_.c_str()) < 0)
    {
        cerr << SystemError(settingsPath_).what() << endl;
    }
    settings_.reset();
    properties();
}


////////////////////////////////////////////////////////////////
void DebuggerBase::save_properties()
{
    Lock<Mutex> lock(settingsMutex_);
    if (settings_)
    {
        ostringstream pid;
        pid << getpid();
        string tmp = settingsPath_ + "." + pid.str();
        try
        {
            { // file stream scope
                FileStream fs(tmp.c_str());
                if (fs.size())
                {
                    fs.truncate();
                }

                const size_t bytes = CHKPTR(settings_)->write(&fs);

                dbgout(0) << "written " << bytes << " byte(s)" << endl;
            }
            if (unlink(settingsPath_.c_str()) < 0)
            {
                cerr << SystemError(settingsPath_).what() << endl;
            }
            else if (rename(tmp.c_str(), settingsPath_.c_str()) < 0)
            {
                cerr << SystemError(tmp).what() << endl;
            }
        }
        catch (const exception& e)
        {
            cerr << settingsPath_ << ": " << e.what() << endl;
        }
    }
}


////////////////////////////////////////////////////////////////
void DebuggerBase::set_options(uint64_t opts)
{
    Lock<Mutex> lock(settingsMutex_);
    options_ = opts;

    if (settings_)
    {
        settings_->set_word("opts", opts);
    }
}


////////////////////////////////////////////////////////////////
uint64_t DebuggerBase::options() const
{
    Lock<Mutex> lock(settingsMutex_);
    return options_;
}


////////////////////////////////////////////////////////////////
void DebuggerBase::throw_signal_out_of_range(const char* func, int signum)
{
    ostringstream err;
    err << func << ": signal " << signum << " is out of range";

    throw out_of_range(err.str());
}


#if DEBUG
namespace std
{
    ostream& operator<<(ostream& os, const pair<string, string>& p)
    {
        os << p.first << ": " << p.second;
        return os;
    }
}
#endif

////////////////////////////////////////////////////////////////
static void read_path_map(map<string, string>& pathMap)
{
    string mapstr = env::get_string("ZERO_PATH_MAP");

    pair<size_t, size_t> key;
    pair<size_t, size_t> val;

    for (;; key.first = val.second + 1)
    {
        key.second = mapstr.find(':', key.first);
        if (key.second == string::npos)
        {
            break;
        }
        val.first = key.second + 1;
        val.second = mapstr.find(';', val.first);
        if (val.second == string::npos)
        {
            break;
        }

        pathMap[mapstr.substr(key.first, key.second - key.first)] =
                mapstr.substr(val.first, val.second - val.first);
    }
#if DEBUG
    copy(pathMap.begin(), pathMap.end(),
            ostream_iterator<pair<string, string> >(cout, "\n"));
#endif
}



////////////////////////////////////////////////////////////////
//
// Resolve symbolic link
//
static bool resolve_link(string& path)
{
#ifdef __unix__
    char link[4096] = { 0 };
    if (readlink(path.c_str(), link, sizeof link - 1) > 0)
    {
    #if DEBUG
        clog << path << " -> " << link << endl;
    #endif
        if (link[0] == '/')
        {
            path = link;
            return true;
        }
        else
        {
            size_t n = path.rfind('/');
            if (n != path.npos)
            {
                path.erase(n + 1, path.npos);
                path += link;
            }
        }
    }
#endif // __unix__
    return false;
}


////////////////////////////////////////////////////////////////
//
// map source code path without following symbolic links
//
bool
DebuggerBase::map_path_no_follow(
    const Process*  proc,
    string&         path) const
{
    bool result = false;
    if (proc)
    {
        if (RefPtr<Target> target = proc->target())
        {
            result = target->map_path(path);
        }
        else
        {
            dbgout(0) << __func__ << ":  target is NULL" << endl;
        }
    }
    if (pathMap_.empty())
    {
        read_path_map(pathMap_);
    }
    PathMap::const_iterator i = pathMap_.lower_bound(path);
    if (i == pathMap_.end() || i->first != path)
    {
        if (i == pathMap_.begin())
        {
            return result;
        }
        --i;
    }
    if ((strncmp(path.c_str(), i->first.c_str(), i->first.size()) == 0)
      &&(strncmp(path.c_str(), i->second.c_str(), i->second.size()) != 0))
    {
        path.replace(0, i->first.size(), i->second);
        result |= true;
    }
    return result; // return false if no mapping occurred
}


////////////////////////////////////////////////////////////////
bool
DebuggerBase::map_path(const Process* proc, string& path) const
{
    bool result = map_path_no_follow(proc, path);
    if (result)
    {
        while (resolve_link(path))
        {
            map_path_no_follow(proc, path);
        }
    }
    dbgout(1) << __func__ << ": " << path << "=" << result << endl;
    return result;
}

////////////////////////////////////////////////////////////////
bool DebuggerBase::check_state_before_attaching(pid_t pid)
{
    TargetPtr target = Target::new_live_target(*this);
    RunnableImpl task(pid, target.get());

    if (task.runstate() == Runnable::ZOMBIE)
    {
        throw runtime_error("cannot attach to defunct process");
    }
    dbgout(1) << __func__ << ": state=" << task.runstate() << endl;

    sys::ptrace(PTRACE_ATTACH, pid, 0, 0);

    bool signalToContinue = false;
    // thread was possibly in a stopped state before attaching
    if (task.runstate() == Runnable::TRACED_OR_STOPPED)
    {
		task.resume();
        signalToContinue = true;
    }
    return signalToContinue;
}

// Copyright (c) 2004 - 2012 Cristian L. Vlasceanu
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
