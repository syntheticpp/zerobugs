#ifndef LINUX_X86_H__2D2CD4D8_7453_4D2E_BF38_AFA6ECEA22C5
#define LINUX_X86_H__2D2CD4D8_7453_4D2E_BF38_AFA6ECEA22C5
//
// $Id: linux-x86.h 714 2010-10-17 10:03:52Z root $
//
// Definitions common to x86-32 and x86-64 systems
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdint.h>

#define GETREGS(regp) 0, reinterpret_cast<word_t>(regp)
#define GETREGS_NATIVE(regp) 0, reinterpret_cast<long>(regp)

// On the x86 software breakpoints are "exceptions",
// i.e.  a signal is raised AFTER the instruction
// is executed (whereas hardware breakpoints are
// "faults", i.e. the signal is raised BEFORE the
// instruction is executed).

#define PROGRAM_COUNT_BACKOUT(pc)  \
    do { assert(pc); --(pc); } while(0)


struct user_regs_32
  {
    uint32_t ebx, ecx, edx, esi, edi, ebp, eax;
    unsigned short ds, __ds, es, __es;
    unsigned short fs, __fs, gs, __gs;
    uint32_t orig_eax, eip;
    unsigned short cs, __cs;
    uint32_t eflags, esp;
    unsigned short ss, __ss;
  };
struct user_fpxregs_32
  {
    unsigned short  cwd;
    unsigned short  swd;
    unsigned short  twd;
    unsigned short  fop;
    uint32_t    fip;
    uint32_t    fcs;
    uint32_t    foo;
    uint32_t    fos;
    uint32_t    mxcsr;
    uint32_t    reserved;
    uint32_t    st_space[32];   // 8*16 bytes for each FP-reg = 128 bytes
    uint32_t    xmm_space[32];  // 8*16 bytes for each XMM-reg = 128 bytes
    uint32_t    padding[56];
 };
#endif // LINUX_X86_H__2D2CD4D8_7453_4D2E_BF38_AFA6ECEA22C5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
