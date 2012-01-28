#ifndef DARWIN_H__D1CB0AF8_4002_4993_9B09_9185F5F72691
#define DARWIN_H__D1CB0AF8_4002_4993_9B09_9185F5F72691
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

#define HAVE_PTRACE_CADDR_T     1

/* true only with minielf emulation */
#define HAVE_ELF_NHDR           1

#define HAVE_THREAD_T           1

#define ADDR_CAST(a) reinterpret_cast<addr_t>(a)

#ifndef __WORDSIZE
 #if (__ppc__) || (__i386__)
  #define __WORDSIZE 32
 #elif __ppc64__
  #define __WORDSIZE 64
 #endif
#endif // __WORDSIZE

#include <sys/ptrace.h>
#include <sys/user.h>
#include <mach/mach.h>

/* fixme: #defines should be the other way around */
#define PTRACE_ATTACH       PT_ATTACH
#define PTRACE_DETACH       PT_DETACH
#define PTRACE_TRACEME      PT_TRACE_ME
#define PTRACE_PEEKDATA     PT_READ_D
#define PTRACE_POKEDATA     PT_WRITE_D
#define PTRACE_PEEKTEXT     PT_READ_I
#define PTRACE_POKETEXT     PT_WRITE_I
#define PTRACE_GETREGS      PT_GETREGS
#define PTRACE_SETREGS      PT_SETREGS
#define PTRACE_GETFPREGS    PT_GETFPREGS
#define PTRACE_SETFPREGS    PT_SETFPREGS
#define PTRACE_GETFPXREGS   PT_GETFPREGS
#define PTRACE_SETFPXREGS   PT_SETFPREGS
#define PTRACE_KILL         PT_KILL
#define PTRACE_CONT         PT_CONTINUE
#define PTRACE_SINGLESTEP   PT_STEP
#define PTRACE_SYSCALL      PT_SYSCALL

#define __WCLONE WLINUXCLONE
#define __WALL   0

typedef int __ptrace_request;

#if __ppc__
 #include <mach/ppc/thread_status.h>

 struct user_regs_struct : public ppc_thread_state
 {
    enum { flavor = PPC_THREAD_STATE };
    enum { count = PPC_THREAD_STATE_COUNT };
 };
#elif __i386__
 #include <mach/i386/thread_status.h>
 struct user_regs_struct : public i386_new_thread_state
 {
    enum { flavor = I386_NEW_THREAD_STATE };
 };
#endif

struct user_fpregs_struct // todo
{ // place-holder
};

struct user_fpxregs_struct // todo
{ // place-holder
};

// TODO brpt 0x7d821008


#if defined(__ppc__) // 32-bit

 #define MAX_FLOAT_BYTES 24 // long double complex

#elif  defined(__ppc64__)

 #define MAX_FLOAT_BYTES 32

#endif

#endif // DARWIN_H__D1CB0AF8_4002_4993_9B09_9185F5F72691
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
