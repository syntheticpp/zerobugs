#ifndef BSD_86_64_H__2FE79592_4E33_11DA_B332_00C04F09BBCC
#define BSD_86_64_H__2FE79592_4E33_11DA_B332_00C04F09BBCC
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

#define INVALID_PC_ADDR 0x0000feed

/* ptrace can read and write into i386 debug registers */
#define HAVE_PTRACE_DBREGS	1

#define GENERAL_REG(r,n) *(&r.r_fs + n)

#define X86REG(s,r) (s).r_##r
#define X86REG_PTR(s,r) (s)->r_##r

#define r_orig_eax r_rax
#define r_xds r_ds
#define r_xes r_es
#define r_xfs r_fs
#define r_xgs r_gs
#define r_xcs r_cs
#define r_xss r_ss
#define r_eflags r_rflags

#define EAX  tEAX
#define EDX  tEDX
#define UESP tESP

/* hack, so that st_space[0] works in the interpreter */
#define st_space fpr_acc

#ifndef __WORDSIZE
 #define __WORDSIZE 64
#endif

#ifndef __LITTLE_ENDIAN
 #define __LITTLE_ENDIAN 1234
#endif

#define MAX_FLOAT_BYTES 24
#define __LIBELF64

//
// for backing out of breakpoints
//
#define PROGRAM_COUNT_BACKOUT(pc)     assert(pc); --(pc)

#endif // BSD_86_64_H__2FE79592_4E33_11DA_B332_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
