//
// $Id: xtrace.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <errno.h>
#include <fcntl.h>
#ifdef __linux__
 #include <linux/unistd.h> // tkill, _syscall2
#endif
#include <signal.h>
#include <stdio.h>
#include "zdk/ref_counted_impl.h"
#include "zdk/ref_ptr.h"
#include "zdk/xtrace.h"
#include <iostream>

using namespace std;


#ifdef HAVE_TKILL_MAYBE
// On newer kernels, kill() sends the signal to all the
// processes in a thread group; we need to kill threads
// individually, use tkill() instead of kill().
#ifndef __NR_tkill
 #define __NR_tkill 238
#endif

#ifdef _syscall2
 _syscall2(int, tkill, pid_t, lwpid, int, signum);
#else
 static inline int tkill(pid_t lwpid, int signum)
 {
     return syscall(__NR_tkill, lwpid, signum);
 }
#endif

int kill_thread(pid_t pid, int sig)
{
    static bool have_tkill = true;

    int result = have_tkill ? tkill(pid, sig) : kill(pid, sig);

    if ((result < 0) && (errno == ENOSYS))
    {
        clog << "tkill() not implemented, using kill instead\n";

        have_tkill = false;
        result = kill(pid, sig);
    }

    return result;
}

#else

int kill_thread(pid_t pid, int sig)
{
    return ::kill(pid, sig);
}

#endif // HAVE_TKILL_MAYBE


XTrace::Services::~Services() throw()
{
}


namespace
{
    CLASS NativeSystemServices : public RefCountedImpl<XTrace::Services>
    {
        pid_t waitpid(pid_t pid, int* status, int options)
        {
            return ::waitpid(pid, status, options);
        }

        long ptrace(__ptrace_request req,
                    pid_t pid,
                    XTrace::Arg addr,
                    XTrace::Arg data)
        {
            return ::ptrace(req, pid, addr.p_, data.i_);
        }

        void exec(char* const argv[], const char* const* env)
        {
            setgid(getpid());
            setsid();
            tcsetpgrp(0, getgid());

            if (const char* input = getenv("ZERO_STDIN"))
            {
                int fd = open(input, O_RDONLY);
                if (fd < 0)
                    cerr << input << ": " << strerror(errno) << endl;
                else
                    dup2(fd, STDIN_FILENO);
            }
            if (const char* output = getenv("ZERO_STDOUT"))
            {
                int fd = open(output, O_RDWR | O_CREAT, 0644);
                if (fd < 0)
                    cerr << output << ": " << strerror(errno) << endl;
                dup2(fd, STDOUT_FILENO);
            }
            while (XTrace::ptrace(PTRACE_TRACEME, 0, 0, 0) < 0
                && errno == EINTR)
            { }

            execve(argv[0], argv, const_cast<char* const*>(env));

            // should not get here if execve()-ed okay, pass
            // errno as exit code to parent
            std::cerr << __func__ << ": errno=" << errno << std::endl;
            _exit(errno);
        }

        int kill(pid_t pid, int nsig)
        {
            return ::kill(pid, nsig);
        }

        int kill_thread(pid_t pid, int nsig)
        {
            return ::kill_thread(pid, nsig);
        }
    };


    CLASS ServiceManager
    {
        RefPtr<XTrace::Services> native_;
        RefPtr<XTrace::Services> srv_;
        mutable Mutex mutex_;

    public:
        ServiceManager() : native_(new NativeSystemServices)
        {
            srv_ = native_;
        }
        RefPtr<XTrace::Services> services() const throw()
        {
            Lock<Mutex> lock(mutex_);
            return srv_;
        }
        void set_services(const RefPtr<XTrace::Services>& srv) throw()
        {
            Lock<Mutex> lock(mutex_);
            srv_ = srv;
        }
    };
} // namespace


static ServiceManager* mgr = new ServiceManager;

RefPtr<XTrace::Services> XTrace::services() throw()
{
    return mgr->services();
}

void XTrace::set_services(const RefPtr<XTrace::Services>& srv) throw()
{
    mgr->set_services(srv);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
