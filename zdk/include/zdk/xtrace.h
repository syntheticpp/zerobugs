#ifndef XTRACE_H__B91D3FF7_3B5C_435A_8177_0E3ABAB4F7AB
#define XTRACE_H__B91D3FF7_3B5C_435A_8177_0E3ABAB4F7AB
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
#include "zdk/config.h"
#ifdef HAVE_SYS_PARAM_H
 #include <sys/param.h>
#endif
#ifdef HAVE_SYS_PTRACE_H
#include <sys/ptrace.h>
#endif
#ifdef HAVE_SYS_WAIT_H
 #include <sys/wait.h>
#endif
#include "zdk/ref_ptr.h"
#include "zdk/unknown2.h"


extern "C" int kill_thread(pid_t, int);



/* XTrace (read Cross-Trace is a level of indirection that abstracts
   out the functionality of ptrace and other low-level debugging functions.
   The intent is to support portability and remote debugging.  */
namespace XTrace
{
#ifdef HAVE_PTRACE_CADDR_T
    typedef caddr_t ptr_t;
#else
    typedef void* ptr_t;
#endif
    union Arg
    {
        ptr_t p_;
        long  i_;

        Arg(ptr_t p) : p_(p) { }
        Arg(long i)  : i_(i) { }
    };

    struct ZDK_EXPORT Services : public RefCounted
    {
        virtual ~Services() throw() = 0;

        virtual pid_t waitpid(pid_t pid, int* status, int options) = 0;

        virtual long ptrace(__ptrace_request, pid_t pid, Arg addr, Arg data) = 0;

        virtual void exec(char* const argv[], const char* const* env) = 0;

        virtual int kill(pid_t, int) = 0;

        virtual int kill_thread(pid_t, int) = 0;
    };


    RefPtr<Services> ZDK_EXPORT services() throw();
    void ZDK_EXPORT set_services(const RefPtr<Services>&) throw();


    inline void ZDK_EXPORT exec(char* const argv[], const char* const* env)
    {
        return services()->exec(argv, env);
    }

    inline pid_t ZDK_EXPORT waitpid(pid_t pid, int* status, int options)
    {
        return services()->waitpid(pid, status, options);
    }

    inline int ZDK_EXPORT kill(pid_t pid, int nsig)
    {
        return services()->kill(pid, nsig);
    }

    inline int ZDK_EXPORT kill_thread(pid_t pid, int nsig)
    {
        return services()->kill_thread(pid, nsig);
    }

    template<typename T, typename U>
    inline long ZDK_EXPORT ptrace(__ptrace_request request, pid_t pid, T addr, U data)
    {
        return services()->ptrace(request, pid, addr, data);
    }
};

#endif // XTRACE_H__B91D3FF7_3B5C_435A_8177_0E3ABAB4F7AB
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
