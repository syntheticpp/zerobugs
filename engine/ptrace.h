#ifndef PTRACE_H__F15DCC88_2448_4BAC_B777_91B645682570
#define PTRACE_H__F15DCC88_2448_4BAC_B777_91B645682570
//
// $Id: ptrace.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#ifdef HAVE_SYS_PTRACE_H
#include <sys/ptrace.h>
#endif
#ifdef HAVE_ASM_PTRACE_H
 #include <asm/ptrace.h>
#endif
#if 0
#ifndef PTRACE_O_TRACESYSGOOD
 #define PTRACE_O_TRACESYSGOOD  0x00000001
#endif

#ifndef PTRACE_O_TRACEFORK

 #define PTRACE_O_TRACEFORK     0x00000002
 #define PTRACE_O_TRACEVFORK    0x00000004
 #define PTRACE_O_TRACECLONE    0x00000008
 #define PTRACE_O_TRACEEXEC     0x00000010
 #define PTRACE_O_TRACEEXIT     0x00000040

 #define PTRACE_EVENT_FORK      1
 #define PTRACE_EVENT_VFORK     2
 #define PTRACE_EVENT_CLONE     3
 #define PTRACE_EVENT_EXEC      4
 #define PTRACE_EVENT_EXIT      6
#endif

#if defined(PTRACE_SETOPTIONS) && (PTRACE_SETOPTIONS == PTRACE_OLDSETOPTIONS)
 #undef PTRACE_SETOPTIONS
#endif
#if !defined(PTRACE_SETOPTIONS) && !defined(PT_SETOPTIONS)
 #define PTRACE_SETOPTIONS      0x4200
#endif
#if !defined(PTRACE_GETEVENTMSG) && !defined(PT_GETEVENTMSG)
 #define PTRACE_GETEVENTMSG     0x4201
#endif
#if !defined(PTRACE_GETSIGINFO) && !defined(PT_GETSIGINFO)
 #define PTRACE_GETSIGINFO      0x4202
#endif
//#if !defined(PTRACE_SETSIGINFO) && !defined(PT_SETSIGINFO)
// #define PTRACE_SETSIGINFO      0x4203
//#endif
#endif
#if !defined(PT_OLDSETOPTIONS)
 #ifndef PTRACE_OLDSETOPTIONS
   #define PTRACE_OLDSETOPTIONS 21
 #endif
 #define PT_OLDSETOPTIONS static_cast<__ptrace_request>(PTRACE_OLDSETOPTIONS)
#endif

#if HAVE___PTRACE_REQUEST
#ifdef PT_SETOPTIONS
 #undef PT_SETOPTIONS
#endif
#define PT_SETOPTIONS static_cast<__ptrace_request>(PTRACE_SETOPTIONS)

#ifdef PT_GETEVENTMSG
 #undef PT_GETEVENTMSG
#endif
#define PT_GETEVENTMSG static_cast<__ptrace_request>(PTRACE_GETEVENTMSG)

#ifdef PT_GETSIGINFO
 #undef PT_GETSIGINFO
#endif
#define PT_GETSIGINFO static_cast<__ptrace_request>(PTRACE_GETSIGINFO)

#else
 #define PT_SETOPTIONS PTRACE_SETOPTIONS
 #define PT_GETEVENTMSG PTRACE_GETEVENTMSG
#endif

#ifndef DEBUG
 #define DEBUG_TRACE_EVENT(tid, s, e)

#else

#define DEBUG_TRACE_EVENT(tid, s, e) \
if ( ((s) >> 16) == (e) ) \
    clog << __func__ << ": " << #e << ": tid=" << (tid) << endl; \
else if ((s) >> 16) \
    clog << __func__<<": "<< hex<<(s)<<dec<<": tid="<<(tid)<< endl; \
else {}

#endif

#endif // PTRACE_H__F15DCC88_2448_4BAC_B777_91B645682570
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
