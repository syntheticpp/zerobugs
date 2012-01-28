#ifndef LINUX_PPC_H__FE047DE0_44B6_4F70_B829_DF8D25F9DF09
#define LINUX_PPC_H__FE047DE0_44B6_4F70_B829_DF8D25F9DF09
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include <sys/procfs.h>
#include <asm/ptrace.h>
#include <sys/ucontext.h>
#include <boost/static_assert.hpp>

#define MAX_FLOAT_BYTES 32 // long double complex

#ifdef  PTRACE_GETREGS
 #undef PTRACE_GETREGS
#endif
#ifdef  PTRACE_SETREGS
 #undef PTRACE_SETREGS
#endif
#ifdef  PTRACE_GETFPREGS
 #undef PTRACE_GETFPREGS
#endif
#ifdef  PTRACE_SETFPREGS
 #undef PTRACE_SETFPREGS
#endif
#define PTRACE_GETREGS    PPC_PTRACE_GETREGS
#define PTRACE_SETREGS    PPC_PTRACE_SETREGS
#define PTRACE_GETFPREGS  PPC_PTRACE_GETFPREGS
#define PTRACE_SETFPREGS  PPC_PTRACE_SETFPREGS

#define PROGRAM_COUNT_BACKOUT(pc) // as nothing

// The PPC_PTRACE_GETREGS option for powerpc is implemented
// so that general purpose registers get copied to the address parameter
// rather than to the data parameter
#define GETREGS(regp) reinterpret_cast<word_t>(regp), 0
#define GETREGS_NATIVE(regp) reinterpret_cast<long>(regp), 0


// on Intel-based platforms, user_regs_struct is defined in /usr/include/sys/user.h
typedef pt_regs user_regs_struct;

typedef fpregset_t user_fpregs_struct;


struct __attribute__((packed)) user_regs_32 : public pt_regs
{
    unsigned long padding[ELF_NGREG - sizeof(pt_regs)/4];
};


struct user_fpxregs_32
{
    // todo
};


#endif // LINUX_PPC_H__FE047DE0_44B6_4F70_B829_DF8D25F9DF09
