#ifndef ARCH_LINUX_H__58FF8B5F_44BB_4663_949D_BD23497973AC
#define ARCH_LINUX_H__58FF8B5F_44BB_4663_949D_BD23497973AC
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
// C++ Templatized versions of elf_prstatus and elf_prpsinfo
//
#include "zdk/config.h"
#include "zdk/export.h"
#include <boost/static_assert.hpp>
#if HAVE_LIBELF_LIBELF_H
  #include <libelf/libelf.h>
#elif HAVE_LIBELF_H
  #include <libelf.h>
#endif
#include <sys/procfs.h>
#include <sys/user.h>
#include <string.h>         // for memset


template<int LongSize> struct Arch
  { };

template<> struct ZDK_LOCAL Arch<32>
  {
#if __WORDSIZE == 32
    typedef long Long;
    typedef unsigned long Ulong;
#else
    typedef int32_t Long;
    typedef uint32_t Ulong;
#endif

#if (__PPC__)
    typedef uint32_t Uid; // UNIX process id
#elif (__i386__) || (__x86_64__)
    typedef uint16_t Uid; // unsigned short
#endif

    typedef uint32_t Addr;
    typedef Elf32_Dyn Dyn;

    typedef Long GeneralRegister;
#if __PPC__
    static const unsigned ngreg = ELF_NGREG;
    BOOST_STATIC_ASSERT(sizeof(user_regs_32)/4 <= ELF_NGREG);
#else
    static const unsigned ngreg = 17;
#endif
    typedef GeneralRegister GeneralRegisterSet[ngreg];

#if (__i386__) || (__PPC__)
    template<typename R> static void
        copy_regs(R& r, const user_regs_32& r32)
        {
            BOOST_STATIC_ASSERT(sizeof(r) == sizeof(r32));
            BUILTIN_MEMCPY(&r, &r32, sizeof(r));
        }
#endif // __i386__
 };



template<> struct ZDK_LOCAL Arch<64>
  {
    typedef int64_t Long;
    typedef uint64_t Ulong;
    typedef uint32_t Uid;
    typedef Ulong Addr;
    typedef Elf64_Dyn Dyn;

    // assume that it is the native platform
    //typedef user_regs_struct GeneralRegisterSet;

    static const unsigned ngreg = ELF_NGREG;
    typedef int64_t GeneralRegister;
    typedef GeneralRegister GeneralRegisterSet[ELF_NGREG];

    template<typename R>
    static void copy_regs(R& regs, const user_regs_32& r32)
    {
#if defined(__x86_64__)
        user_regs_struct& r = reinterpret_cast<user_regs_struct&>(regs);
        r.rbx = r32.ebx;
        r.rcx = r32.ecx;
        r.rdx = r32.edx;
        r.rsi = r32.esi;
        r.rdi = r32.edi;
        r.rbp = r32.ebp;
        r.rax = r32.eax;
        r.ds = r32.ds;
        r.es = r32.es;
        r.fs = r32.fs;
        r.gs = r32.gs;
        r.cs = r32.cs;
        r.rip = r32.eip;
        r.eflags = r32.eflags;
        r.rsp = r32.esp;
        r.ss = r32.ss;
#endif //__x86_64__
    }
  };


template<int W>
struct ZDK_LOCAL elf_prstatus_
{
    typedef typename Arch<W>::Ulong Ulong;

    struct Timeval
    {
        typename Arch<W>::Long tv_sec;
        typename Arch<W>::Long tv_usec;
    };

    struct elf_siginfo pr_info; // Info associated with signal.
    short int pr_cursig;        // Current signal.
    Ulong pr_sigpend;           // Set of pending signals.
    Ulong pr_sighold;           // Set of held signals.
    __pid_t pr_pid;
    __pid_t pr_ppid;
    __pid_t pr_pgrp;
    __pid_t pr_sid;

    Timeval pr_utime;           // User time.
    Timeval pr_stime;           // System time.
    Timeval pr_cutime;          // Cumulative user time.
    Timeval pr_cstime;          // Cumulative system time.

    typename Arch<W>::GeneralRegisterSet pr_reg;
    int pr_fpvalid;             // True if math copro being used.

    elf_prstatus_()
        : pr_cursig(0)
        , pr_sigpend(0)
        , pr_sighold(0)
        , pr_pid(0)
        , pr_ppid(0)
        , pr_pgrp(0)
        , pr_sid(0)
        , pr_fpvalid(0)
    {
        memset(&pr_info, 0, sizeof(pr_info));
        memset(&pr_utime, 0, sizeof(pr_utime));
        memset(&pr_stime, 0, sizeof(pr_stime));
        memset(&pr_cutime, 0, sizeof(pr_cutime));
        memset(&pr_cstime, 0, sizeof(pr_cstime));
        memset(&pr_reg, 0, sizeof(pr_reg));
    }

    /**
     * For cross-debugging 32-on-64
     */
    template<int X>
    elf_prstatus_(const elf_prstatus_<X>& x)
        : pr_cursig(x.pr_cursig)
        , pr_sigpend(x.pr_sigpend)
        , pr_sighold(x.pr_sighold)
        , pr_pid(x.pr_pid)
        , pr_ppid(x.pr_ppid)
        , pr_pgrp(x.pr_pgrp)
        , pr_sid(x.pr_sid)
        , pr_fpvalid(x.pr_fpvalid)
    {
        BUILTIN_MEMCPY(&pr_info, &x.pr_info, sizeof(pr_info));
        BUILTIN_MEMCPY(&pr_utime, &x.pr_utime, sizeof(pr_utime));
        BUILTIN_MEMCPY(&pr_stime, &x.pr_stime, sizeof(pr_stime));
        BUILTIN_MEMCPY(&pr_cutime, &x.pr_cutime, sizeof(pr_cutime));
        BUILTIN_MEMCPY(&pr_cstime, &x.pr_cstime, sizeof(pr_cstime));

        Arch<W>::copy_regs(pr_reg, reinterpret_cast<const user_regs_32&>(x.pr_reg));
    }

    operator prstatus_t& ()
    {
        BOOST_STATIC_ASSERT(sizeof (prstatus_t) == sizeof (*this));
        return reinterpret_cast<prstatus_t&>(*this);
    }
    operator const prstatus_t& () const
    {
        BOOST_STATIC_ASSERT(sizeof (prstatus_t) == sizeof (*this));
        return reinterpret_cast<const prstatus_t&>(*this);
    }
};


template<int W = __WORDSIZE>
struct ZDK_LOCAL elf_prpsinfo_
  {
    typedef typename Arch<W>::Ulong Ulong;
    typedef typename Arch<W>::Uid Uid;

    char pr_state;          // Numeric process state.
    char pr_sname;          // Char for pr_state.
    char pr_zomb;           // Zombie.
    char pr_nice;           // Nice val.
    Ulong pr_flag;          // Flags.

    Uid pr_uid;
    Uid pr_gid;

    int pr_pid, pr_ppid, pr_pgrp, pr_sid;

    char pr_fname[16];          // Filename of executable.
    char pr_psargs[ELF_PRARGSZ];// Initial part of arg list.


    template<int X>
        elf_prpsinfo_(const elf_prpsinfo_<X>& x)
            : pr_state(x.pr_state)
            , pr_sname(x.pr_sname)
            , pr_zomb(x.pr_zomb)
            , pr_nice(x.pr_nice)
            , pr_flag(x.pr_flag)
            , pr_uid(x.pr_uid)
            , pr_gid(x.pr_gid)
            , pr_pid(x.pr_pid)
            , pr_ppid(x.pr_ppid)
            , pr_pgrp(x.pr_pgrp)
            , pr_sid(x.pr_sid)
    {
        BUILTIN_MEMCPY(pr_fname, x.pr_fname, sizeof pr_fname);
        BUILTIN_MEMCPY(pr_psargs, x.pr_psargs, sizeof pr_psargs);
    }
  };
BOOST_STATIC_ASSERT(sizeof (elf_prpsinfo_<>) == sizeof(elf_prpsinfo));



template<int W = __WORDSIZE>
struct ZDK_LOCAL link_map_
  {
    typedef typename Arch<W>::Addr Addr;
    /* These first few members are part of the protocol with the debugger.
       This is the same format used in SVR4.  */

    Addr l_addr;        // Base address shared object is loaded at.
    Addr l_name;        // Absolute file name object was found in.
    Addr l_ld;          // Dynamic section of the shared object.
    Addr l_next, l_prev;// Chain of loaded objects.

    link_map_() { memset(this, 0, sizeof (*this)); };

    template<int X>
        link_map_(const link_map_<X>& x)
        : l_addr(x.l_addr)
        , l_name(x.l_name)
        , l_ld(x.l_ld)
        , l_next(x.l_next)
        , l_prev(x.l_prev)
    { }
 };


enum r_state_enum
  {
    /* This state value describes the mapping change taking place when
       the `r_brk' address is called.  */
    RT_CONSISTENT,          // Mapping change is complete.
    RT_ADD,                 // Beginning to add a new object.
    RT_DELETE               // Beginning to remove an object mapping.
  };

/* Rendezvous structure used by the run-time dynamic linker to communicate
   details of shared object loading to the debugger.  If the executable's
   dynamic section has a DT_DEBUG element, the run-time linker sets that
   element's value to the address where this structure can be found.  */

template<int W = __WORDSIZE>
struct ZDK_LOCAL r_debug_
  {
    int r_version;                  // Version number for this protocol.

    typename Arch<W>::Addr r_map;  // Head of the chain of loaded objects.

    /* This is the address of a function internal to the run-time linker,
       that will always be called when the linker begins to map in a
       library or unmap it, and again when the mapping change is complete.
       The debugger can set a breakpoint at this address if it wants to
       notice shared object mapping changes.  */
    typename Arch<W>::Addr r_brk;

    r_state_enum r_state;

    typename Arch<W>::Addr r_ldbase;// Base address the linker is loaded at.

    r_debug_() { memset(this, 0, sizeof(*this)); }

    template<int X>
        r_debug_(const r_debug_<X>& x)
        : r_version(x.r_version)
        , r_map(x.r_map)
        , r_brk(x.r_brk)
        , r_state(x.r_state)
        , r_ldbase(x.r_ldbase)
    { }
  };

#endif // ARCH_LINUX_H__58FF8B5F_44BB_4663_949D_BD23497973AC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
