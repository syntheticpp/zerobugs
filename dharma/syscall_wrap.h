#ifndef SYSCALL_WRAP_H__B2950998_051F_47A9_8F02_31A055DBAB15
#define SYSCALL_WRAP_H__B2950998_051F_47A9_8F02_31A055DBAB15
//
// $Id$
//
// Miscellaneous system call wrappers
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
#include "zdk/xtrace.h"
#include <sys/stat.h>
#ifdef HAVE_SYS_USER_H
 #include <sys/user.h>  // user_regs_struct
#endif
#include <new>          // for std::nothrow
#include <string>
#include "zdk/platform.h"

using Platform::addr_t;
using Platform::word_t;

#ifdef __MACH__
    #include "syscall_mach.h"
#elif defined(__unix__)
    #include "syscall_unix.h"
#endif
#if (__GNUC__ >= 4)
    #pragma GCC visibility push (hidden)
#endif

namespace sys
{
    // for troubleshooting ptrace calls, when the ZERO_PTRACE_ERROR
    // environment variable is set to -2
    typedef bool ptrace_error_handler(int err);

    /**
     * @note NOT thread-safe
     */
    void set_ptrace_error_handler(ptrace_error_handler*);

    size_t ptrace(__ptrace_request, pid_t, addr_t addr, word_t data = 0);

    void kill(pid_t tid, int sigNum);

    void kill_thread(pid_t tid, int sigNum);
    bool kill_thread(pid_t tid, int sigNum, std::nothrow_t) throw();

    pid_t waitpid(pid_t, int*, int);

    pid_t waitpid(pid_t, int*, int, std::nothrow_t) throw();

    pid_t fork();

    void mkdir(const std::string&, mode_t);

    void rmdir(const std::string&, bool deleteRecursive = false);

    void stat(const std::string&, struct stat&);

    int open(const char*, int flags);

    void read(int fd, void *buf, size_t count);
    void write(int fd, const void *buf, size_t count);
    void write(FILE*, const void *buf, size_t count);

    loff_t lseek(int fd, loff_t offset, int whence);

    /**
     * get configuration dependent string variables, see confstr(3)
     */
    std::string confstr(int name);

    bool uses_nptl();

    void unmask_all_signals();

    class ZDK_LOCAL ImpersonationScope
    {
    public:
        explicit ImpersonationScope(uid_t);
        ~ImpersonationScope();

    private:
        //non-copyable, non-assignable
        ImpersonationScope(const ImpersonationScope&);
        ImpersonationScope& operator=(const ImpersonationScope&);

        uid_t  euid_;  // effective user id
    };
}

#if (__GNUC__ >= 4)
    #pragma GCC visibility pop
#endif


#endif // SYSCALL_WRAP_H__B2950998_051F_47A9_8F02_31A055DBAB15
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
