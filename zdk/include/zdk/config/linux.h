#ifndef LINUX_H__F9B08446_4E32_11DA_B332_00C04F09BBCC
#define LINUX_H__F9B08446_4E32_11DA_B332_00C04F09BBCC
//
// $Id: linux.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <sys/sysinfo.h>

#if defined(__GNUC__) && ((__GNUC__ < 3) || !defined(_ISOC99_SOURCE))
   #define strtold strtod
   #define fabsl   fabs
#endif

#define HAVE_ELF                1

#define HAVE_CLONE              1

#define HAVE_PTRACE_PEEKUSER    1

#define HAVE_PTRACE_POKEUSER    1

#define HAVE___PTRACE_REQUEST   1

#define HAVE_PRSTATUS_PPID      1

#define HAVE_PRSTATUS_PGRP      1

#define HAVE_PRPSINFO_PID       1

#define HAVE_SIGPOLL            1

#define HAVE_SIGPWR             1

#define HAVE_SIGSTKFLT          1

#define HAVE_ELF32_XWORD        1
#define HAVE_ELF64_XWORD        1

#define HAVE_ELF32_SXWORD       1
#define HAVE_ELF64_SXWORD       1

#define HAVE_ELF_NHDR           1

#define HAVE_XMM                1 // used in Dwarf ABI
#define ADDR_CAST(a) (a)

#define GENERAL_REG(r,n) r[n]

//
// special invalid address pushed on the stack by the
// expression interpreter, to signal return from a function call
//
#define INVALID_PC_ADDR 0x0000feed

#include <endian.h>

#ifdef __i386__
 #include "zdk/config/linux-i386.h"
 #ifndef __WORDSIZE
  #define __WORDSIZE 32
 #endif

#elif defined(__x86_64__)

 #include "zdk/config/linux-x86_64.h"
 #ifndef __WORDSIZE
  #define __WORDSIZE 64
 #endif

#elif defined (__PPC__)
 #include "zdk/config/linux-ppc.h"

#else
 #error unsupported platform
#endif

#endif // LINUX_H__F9B08446_4E32_11DA_B332_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
