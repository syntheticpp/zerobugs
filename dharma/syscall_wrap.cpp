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

#include "zdk/config.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_SYS_PARAM_H
 #include <sys/param.h>
#endif
#include <sys/user.h>
#include <sstream>
#include <iostream>
#include "dharma/directory.h"
#include "dharma/environ.h"
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"

using namespace std;


ZDK_EXPORT sys::ptrace_error_handler* handler = 0;

void sys::set_ptrace_error_handler(ptrace_error_handler* h)
{
    handler = h;
}


static void
ptrace_throw_error(__ptrace_request rq, pid_t pid, addr_t addr, word_t data)
{
    if (rq != PTRACE_TRACEME)
    {
        // construct a verbose error message
        ostringstream errmsg;

        errmsg << "ptrace(" << rq << ", pid=" << pid
               << ", addr=0x" << hex << addr
               << ", data=0x" << data << dec << ")";
        throw SystemError(errmsg.str(), errno, false);
    }
    throw SystemError("ptrace");
}


static bool ptrace_handle_error(__ptrace_request rq)
{
    // environment variables for debugging ptrace failures:
    // can force a core dump
    static const int ERR = env::get("ZERO_PTRACE_ERR", 0);
    static const int REQ = env::get("ZERO_PTRACE_REQ", -1);

    if ((ERR == -1 || ERR == errno) && (REQ == -1 || REQ == rq))
    {
        abort();
    }
    if ((ERR == -2) && handler)
    {
        if (handler(errno))
        {
            return true;
        }
    }
    return false;
}



size_t
sys::ptrace(__ptrace_request rq, pid_t pid, addr_t addr, word_t data)
{
    errno = 0;
    long res = 0;

    for (;;)
    {
        res = XTrace::ptrace(rq, pid, addr, data);

        if ((res == -1) && (errno != 0))
        {
            if (errno == EINTR)
            {
                continue;
            }
            if (!ptrace_handle_error(rq))
            {
                ptrace_throw_error(rq, pid, addr, data);
            }
        }
        break;
    }
    return static_cast<size_t>(res);
}


void sys::kill_thread(pid_t pid, int sigNum)
{
    while (XTrace::kill_thread(pid, sigNum) < 0)
    {
        if (errno != EINTR)
        {
            throw SystemError("kill_thread");
        }
    }
}


bool sys::kill_thread(pid_t pid, int sigNum, nothrow_t) throw()
{
    while (XTrace::kill_thread(pid, sigNum) < 0)
    {
        if (errno != EINTR)
        {
            return false;
        }
    }
    return true;
}


void sys::kill(pid_t pid, int sigNum)
{
    while (XTrace::kill(pid, sigNum) < 0)
    {
        if (errno != EINTR)
        {
            throw SystemError(__func__);
        }
    }
}


pid_t sys::waitpid(pid_t pid, int* status, int options)
{
    pid_t rpid = 0;
    for (;;)
    {
        rpid = XTrace::waitpid(pid, status, options);

        if (rpid < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            ostringstream err;
            err << "waitpid(" << pid << ")";
            throw SystemError(err.str());
        }
        assert(rpid || (options & WNOHANG));
        break;
    }
    return rpid;
}


pid_t
sys::waitpid(pid_t pid, int* status, int options, nothrow_t) throw()
{
    pid_t rpid = 0;
    for (;;)
    {
        rpid = XTrace::waitpid(pid, status, options);

        if (rpid < 0 && errno == EINTR)
        {
            continue;
        }
        break;
    }
    return rpid;
}


pid_t sys::fork()
{
    pid_t pid = 0;

    while ((pid = ::fork()) < 0)
    {
        if (errno != EINTR)
        {
            throw SystemError("fork");
        }
    }
    return pid;
}


void sys::stat(const string& path, struct stat& stbuf)
{
    memset(&stbuf, 0, sizeof stbuf);

    while (::stat(path.c_str(), &stbuf) < 0)
    {
        if (errno != EINTR)
        {
            throw SystemError("could not stat " + path);
        }
    }
}


void sys::mkdir(const string& path, mode_t mode)
{
    while (::mkdir(path.c_str(), mode) < 0)
    {
        if (EINTR == errno)
        {
            continue; // interrupted, retry
        }
        if (EEXIST == errno)
        {
            struct stat stbuf;
            sys::stat(path, stbuf);
            if (S_ISDIR(stbuf.st_mode))
            {
                break; // exists and is a directory, fine
            }
            // exists and it is not a directory, give up
            throw SystemError("mkdir: " + path, EEXIST);
        }
        // give up on all other errors
        throw SystemError("mkdir: " + path, errno);
    }
}


void sys::rmdir(const string& path, bool deleteRecursive)
{
    if (!deleteRecursive)
    {
        while(::rmdir(path.c_str()) < 0)
        {
            if (errno != EINTR)
            {
                throw SystemError("rmdir: " + path, errno);
            }
        }
    }
    else
    {
        Directory dir(path, NULL, true);

        if (!dir.empty())
        {
            Directory::const_iterator i = dir.end();
            do
            {
                --i;
                if (i.short_path() == "." || i.short_path() == "..")
                {
                    continue;
                }

                struct stat stbuf;
                stat(*i, stbuf);

                if (S_ISDIR(stbuf.st_mode))
                {
                    sys::rmdir(*i, false);
                }
                else
                {
                    while (::unlink((*i).c_str()) < 0)
                    {
                        if (errno != EINTR)
                        {
                            throw SystemError("rmdir: could not unlink " + *i);
                        }
                    }
                }
            } while (i != dir.begin());
        }
        rmdir(path, false);
    }
}



string sys::confstr(int name)
{
    string ret;

    // determine the length
    if (size_t len = ::confstr(name, NULL, 0))
    {
        vector<char> buf(len);
        if (::confstr(name, &buf[0], len))
        {
            ret.assign(&buf[0]);
        }
    }
    else if (errno)
    {
        throw SystemError(__func__, errno);
    }
    return ret;
}


bool sys::uses_nptl()
{
#ifdef __linux__
    static bool usesNPTL =
        sys::confstr(_CS_GNU_LIBPTHREAD_VERSION).find("NPTL") == 0;

    return usesNPTL;
#else
    return false;
#endif
}


int sys::open(const char* filename, int flags)
{
    int fd = -1;

    while (true)
    {
        fd = ::open(filename, flags, 0664);

        if (fd < 0)
        {
            if (errno == EINTR) continue;
            throw SystemError(filename, errno);
        }
        break;
    }
    return fd;
}


void sys::write(FILE* f, const void *buf, size_t count)
{
    assert(f);
    for (size_t bytesWritten = 0;;)
    {
        bytesWritten = ::fwrite(buf, 1, count, f);

        if (bytesWritten < count)
        {
            if (errno == EINTR) continue;
            throw SystemError(__func__, errno);
        }
        break;
    }
}


void sys::write(int fd, const void *buf, size_t count)
{
    for (ssize_t bytesWritten = 0; /* errno = 0 */;)
    {
        bytesWritten = ::write(fd, buf, count);

        if (bytesWritten < 0)
        {
            if (errno == EINTR) continue;
            throw SystemError(__func__, errno);
        }
        else if (size_t(bytesWritten) != count)
        {
            throw SystemError(__func__, errno);
        }
        break;
    }
}


void sys::read(int fd, void *buf, size_t count)
{
    for (ssize_t bytesRead = 0; /* errno = 0 */;)
    {
        bytesRead = ::read(fd, buf, count);

        if (bytesRead < 0)
        {
            if (errno == EINTR) continue;
            throw SystemError(__func__, errno);
        }
        else if (size_t(bytesRead) != count)
        {
            throw SystemError(__func__, errno);
        }
        break;
    }
}


loff_t sys::lseek(int fd, loff_t offset, int whence)
{
    loff_t pos = 0;

    for (;;)
    {
        pos = lseek64(fd, offset, whence);
        if ((pos < 0) && (errno != 0))
        {
            if (errno == EINTR)
            {
                continue;
            }
            throw SystemError(__func__);
        }
        break;
    }
    return pos;
}


void sys::unmask_all_signals()
{
    sigset_t mask;

    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);
}


sys::ImpersonationScope::ImpersonationScope(uid_t uid)
    // save current effective user id
    : euid_(geteuid())
{
    // impersonate uid
    if (seteuid(uid) < 0)
    {
        throw SystemError("seteuid");
    }
    // clog << "Effective user id=" << geteuid() << endl;
}


sys::ImpersonationScope::~ImpersonationScope()
{
    // restore to save effective uid
    seteuid(euid_);

    // clog << "Restored effective user id=" << geteuid() << endl;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
