#ifndef XTRACE_H__B91D3FF7_3B5C_435A_8177_0E3ABAB4F7AB
#define XTRACE_H__B91D3FF7_3B5C_435A_8177_0E3ABAB4F7AB
//
// $Id: xtrace.h 723 2010-10-28 07:40:45Z root $
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


// Hmm... This looks like a very bad idea
#if 0 // !(HAVE_SYS_PTRACE_H)

/* Type of the REQUEST argument to `ptrace.' If ptrace is not defined
   on this system, fallback to the Linux definition of __ptrace_request. */
enum __ptrace_request
{
  /* Indicate that the process making this request should be traced.
     All signals received by this process can be intercepted by its
     parent, and its parent can use the other `ptrace' requests.  */
  PTRACE_TRACEME = 0,
#define PT_TRACE_ME PTRACE_TRACEME

  /* Return the word in the process's text space at address ADDR.  */
  PTRACE_PEEKTEXT = 1,
#define PT_READ_I PTRACE_PEEKTEXT

  /* Return the word in the process's data space at address ADDR.  */
  PTRACE_PEEKDATA = 2,
#define PT_READ_D PTRACE_PEEKDATA

  /* Return the word in the process's user area at offset ADDR.  */
  PTRACE_PEEKUSER = 3,
#define PT_READ_U PTRACE_PEEKUSER

  /* Write the word DATA into the process's text space at address ADDR.  */
  PTRACE_POKETEXT = 4,
#define PT_WRITE_I PTRACE_POKETEXT

  /* Write the word DATA into the process's data space at address ADDR.  */
  PTRACE_POKEDATA = 5,
#define PT_WRITE_D PTRACE_POKEDATA

  /* Write the word DATA into the process's user area at offset ADDR.  */
  PTRACE_POKEUSER = 6,
#define PT_WRITE_U PTRACE_POKEUSER

  /* Continue the process.  */
  PTRACE_CONT = 7,
#define PT_CONTINUE PTRACE_CONT

  /* Kill the process.  */
  PTRACE_KILL = 8,
#define PT_KILL PTRACE_KILL

  /* Single step the process.
     This is not supported on all machines.  */
  PTRACE_SINGLESTEP = 9,
#define PT_STEP PTRACE_SINGLESTEP

  /* Get all general purpose registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETREGS = 12,
#define PT_GETREGS PTRACE_GETREGS

  /* Set all general purpose registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETREGS = 13,
#define PT_SETREGS PTRACE_SETREGS

  /* Get all floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETFPREGS = 14,
#define PT_GETFPREGS PTRACE_GETFPREGS

  /* Set all floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETFPREGS = 15,
#define PT_SETFPREGS PTRACE_SETFPREGS

  /* Attach to a process that is already running. */
  PTRACE_ATTACH = 16,
#define PT_ATTACH PTRACE_ATTACH

  /* Detach from a process attached to with PTRACE_ATTACH.  */
  PTRACE_DETACH = 17,
#define PT_DETACH PTRACE_DETACH

  /* Get all extended floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETFPXREGS = 18,
#define PT_GETFPXREGS PTRACE_GETFPXREGS

  /* Set all extended floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETFPXREGS = 19,
#define PT_SETFPXREGS PTRACE_SETFPXREGS

  /* Continue and stop at the next (return from) syscall.  */
  PTRACE_SYSCALL = 24,
#define PT_SYSCALL PTRACE_SYSCALL

  /* Set ptrace filter options.  */
  PTRACE_SETOPTIONS = 0x4200,
#define PT_SETOPTIONS PTRACE_SETOPTIONS

  /* Get last ptrace message.  */
  PTRACE_GETEVENTMSG = 0x4201,
#define PT_GETEVENTMSG PTRACE_GETEVENTMSG

  /* Get siginfo for process.  */
  PTRACE_GETSIGINFO = 0x4202,
#define PT_GETSIGINFO PTRACE_GETSIGINFO

  /* Set new siginfo for process.  */
  PTRACE_SETSIGINFO = 0x4203
#define PT_SETSIGINFO PTRACE_SETSIGINFO
};
#endif // !(HAVE_SYS_PTRACE_H)



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
