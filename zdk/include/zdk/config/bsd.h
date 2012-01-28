#ifndef BSD_H__CD4770C7_4E32_11DA_B332_00C04F09BBCC
#define BSD_H__CD4770C7_4E32_11DA_B332_00C04F09BBCC
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

#include <boost/static_assert.hpp>

#define HAVE_ELF                1

#define HAVE_PROC_SERVICE_H     1

#define HAVE_THREAD_DB_H        1

/* address type in link_map struct is caddr_t */
#define HAVE_LINK_MAP_CADDR_T   1

#define HAVE_PRSTATUS_GREGSETSZ 1

#define HAVE_PTRACE_CADDR_T     1

/* ptrace can set and read the reg struct */
#define HAVE_PTRACE_REGS        1

#define HAVE_PT_CLEARSTEP       1

#define HAVE_KSE_THREADS        1

#define ADDR_CAST(a) reinterpret_cast<const addr_t&>(a)

/* true when using our very own libelf.h (minielf) */
#define HAVE_ELF_NHDR           1

#if defined(__i386__)
 #include "zdk/config/bsd386.h"
#elif defined(__x86_64__)
 #include "zdk/config/bsd-x86_64.h"
#endif

/* compatibility */
#include <sys/types.h>
#include <machine/ptrace.h>
#include <machine/reg.h>
#include <sys/signal.h>

typedef sig_t sighandler_t;

/* PT_GETREGS, PT_GETFPREGS etc expect the
   register struct in the address field */

#define GETREGS(regp) reinterpret_cast<addr_t>(regp), 0

#define GETREGS_NATIVE(regp) reinterpret_cast<caddr_t>(regp), 0

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

typedef struct reg user_regs_struct;
typedef struct fpreg user_fpregs_struct;
typedef struct prstatus elf_prstatus;
typedef struct prpsinfo elf_prpsinfo;

BOOST_STATIC_ASSERT(sizeof(off_t) == 8);
typedef off_t loff_t;

inline loff_t lseek64(int fd, loff_t offset, int whence)
{
    return lseek(fd, offset, whence);
}


#endif // BSD_H__CD4770C7_4E32_11DA_B332_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
