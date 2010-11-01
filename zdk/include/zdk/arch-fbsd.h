#ifndef ARCH_FBSD_H__5F9C65D4_FDF8_11DB_8EB2_00C04F09BBCC
#define ARCH_FBSD_H__5F9C65D4_FDF8_11DB_8EB2_00C04F09BBCC
//
// $Id: arch-fbsd.h 719 2010-10-22 03:59:11Z root $
//
// C++ Templetized versions of elf_prstatus and elf_prpsinfo
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#ifdef HAVE_SYS_PARAM_H
 #include <sys/param.h>
#endif
#include <machine/reg.h>
//#include <machine/vm86.h>
#include <boost/static_assert.hpp>
//#include <libelf/libelf.h>
#include <sys/procfs.h>
//#include <sys/user.h>

//#ifndef ELF_PRARGSZ
// #define ELF_PRARGSZ 32 // fixme
//#endif

typedef struct reg gregset_t;
typedef struct fpreg fpregset_t;
typedef struct reg32 user_regs_32; // XXX

template<int LongSize> struct Arch
  { };

template<> struct Arch<32>
  {
#ifdef __i386__
    typedef long Long;
    typedef unsigned long Ulong;
#else
    typedef int32_t Long;
    typedef uint32_t Ulong;
#endif
    typedef uint16_t Uid;

    typedef Ulong Addr;
    typedef Elf32_Dyn Dyn;

    static const unsigned ngreg = sizeof(struct reg)/sizeof(unsigned);

#ifdef __i386__
    template<typename R> static void
        copy_regs(R& r, const user_regs_32& r32)
        {
            BOOST_STATIC_ASSERT(sizeof(r) == sizeof(r32));
            memcpy(&r, &r32, sizeof(r));
        }
#endif // __i386__
 };

template<> struct Arch<64>
  {
    typedef int64_t Long;
    typedef uint64_t Ulong;
    typedef uint32_t Uid;
    typedef Ulong Addr;
    typedef Elf64_Dyn Dyn;

    // assume that it is the native platform
    // static const unsigned ngreg = ELF_NGREG;
/*
    template<typename R>
    static void copy_regs(R& regs, const user_regs_32& r32)
    {
#if __x86_64__
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
    */
  };

template<int W>
struct elf_prstatus_
{
    int		pr_version;	/* Version number of struct (1) */
    size_t	pr_statussz;	/* sizeof(prstatus_t) (1) */
    size_t	pr_gregsetsz;	/* sizeof(gregset_t) (1) */
    size_t	pr_fpregsetsz;	/* sizeof(fpregset_t) (1) */
    int		pr_osreldate;	/* Kernel version (1) */
    int		pr_cursig;	/* Current signal (1) */
    pid_t	pr_pid;		/* Process ID (1) */
    gregset_t	pr_reg;		/* General purpose registers (1) */

    template<int V>
    operator elf_prstatus_<V>& ()
    {
        //BOOST_STATIC_ASSERT(sizeof (prstatus_t) == sizeof (*this));
        //return reinterpret_cast<elf_prstatus_<V>&>(*this);
        return static_cast<elf_prstatus_<V>&>(*this);
    }
    template<int V>
    operator const elf_prstatus_<V>& () const
    {
        //BOOST_STATIC_ASSERT(sizeof (prstatus_t) == sizeof (*this));
        //return reinterpret_cast<const elf_prstatus_<V>&>(*this);
        return static_cast<const elf_prstatus_<V>&>(*this);
    }
};

//typedef gregset_t prgregset_t[1];
//typedef fpregset_t prfpregset_t;

//#define PRFNAMESZ	16	/* Maximum command length saved */
//#define PRARGSZ		80	/* Maximum argument bytes saved */

template<int W>
struct elf_prpsinfo_ {
    int		pr_version;	/* Version number of struct (1) */
    size_t	pr_psinfosz;	/* sizeof(prpsinfo_t) (1) */
    char	pr_fname[PRFNAMESZ+1];	/* Command name, null terminated (1) */
    char	pr_psargs[PRARGSZ+1];	/* Arguments, null terminated (1) */
    template<int V>
    operator elf_prpsinfo_<V>& ()
    {
        return static_cast<elf_prpsinfo_<V>&>(*this);
    }
    template<int V>
    operator const elf_prpsinfo_<V>& () const
    {
        return static_cast<const elf_prpsinfo_<V>&>(*this);
    }
} ;


template<int W = __WORDSIZE>
struct link_map_
  {
    typedef typename Arch<W>::Addr Addr;
    /* These first few members are part of the protocol with the debugger.
       This is the same format used in SVR4.  */

    Addr l_addr;        // Base address shared object is loaded at.
    Addr l_name;        // Absolute file name object was found in.
    Addr l_ld;          // Dynamic section of the shared object.
    Addr l_next, l_prev; // Chain of loaded objects.

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
struct r_debug_
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

#endif // ARCH_FBSD_H__5F9C65D4_FDF8_11DB_8EB2_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
