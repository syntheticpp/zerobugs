#ifndef ABI_X86_H__8EE3A891_5B3E_4F34_9EE9_34ECEDF5036F
#define ABI_X86_H__8EE3A891_5B3E_4F34_9EE9_34ECEDF5036F
//
// $Id: abi_x86.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/config.h"
#include <sys/param.h>
#include <sys/user.h>
#include "addr_ops.h"


struct RegInfo
{
    long r_user; // register index, or offset
                 // in user_fpregs_struct if negative
    const char* r_name;
};

namespace Dwarf
{
    namespace ABI
    {
        void out_of_range(const char* fun, size_t n);
    }
}


#define OFFSET_OF(u,x) (long)(&reinterpret_cast<u*>(0)->x)
#define REG_OFFSET(u,x) (long)(&X86REG_PTR((u*)0, x ))

#define DWARF_REG_MAP(n,x) { \
    (REG_OFFSET(user_regs_struct,x)/sizeof(long)), #x }


#if defined (__i386__)
#define DWARF_FPU_REG(n,x) { -(OFFSET_OF(user_fpxregs_struct,x)), #x }
//
// SVR4 reference port C compiler Dwarf ABI
//
static const RegInfo regMap[] =
{
    DWARF_REG_MAP(0, eax),
    DWARF_REG_MAP(1, ecx),
    DWARF_REG_MAP(2, edx),
    DWARF_REG_MAP(3, ebx),
    DWARF_REG_MAP(4, esp),
    DWARF_REG_MAP(5, ebp),
    DWARF_REG_MAP(6, esi),
    DWARF_REG_MAP(7, edi),
    DWARF_REG_MAP(8, eip),
    DWARF_REG_MAP(9, eflags),
    DWARF_REG_MAP(10, orig_eax), // any better idea?

    DWARF_FPU_REG(11, st_space[0]),
    DWARF_FPU_REG(12, st_space[4]),
    DWARF_FPU_REG(13, st_space[8]),
    DWARF_FPU_REG(14, st_space[12]),
    DWARF_FPU_REG(15, st_space[16]),
    DWARF_FPU_REG(16, st_space[20]),
    DWARF_FPU_REG(17, st_space[24]),
    DWARF_FPU_REG(18, st_space[28]),
};


static const size_t MAP_SIZE_32 = sizeof(regMap)/sizeof(regMap[0]);
static const RegInfo (&regMap32) [MAP_SIZE_32] = regMap;


////////////////////////////////////////////////////////////////
#else // x86-64 ABI
//
// hack: use negative offsets for the floating point regs
//
#define DWARF_FPU_REG(n,x) { -(REG_OFFSET(user_fpregs_struct,x)), #x }
//
// http://www.genunix.org/wiki/index.php/Dwarf_Register_Numbering
// http://www.x86-64.org/documentation/abi.pdf
//
static const RegInfo regMap[] =
{
    DWARF_REG_MAP(0, rax),
    DWARF_REG_MAP(1, rdx),
    DWARF_REG_MAP(2, rcx),
    DWARF_REG_MAP(3, rbx),
    DWARF_REG_MAP(4, rsi),
    DWARF_REG_MAP(5, rdi),
    DWARF_REG_MAP(6, rbp),
    DWARF_REG_MAP(7, rsp),
    DWARF_REG_MAP(8, r8),
    DWARF_REG_MAP(9, r9),
    DWARF_REG_MAP(10, r10),
    DWARF_REG_MAP(11, r11),
    DWARF_REG_MAP(12, r12),
    DWARF_REG_MAP(13, r13),
    DWARF_REG_MAP(14, r14),
    DWARF_REG_MAP(15, r15),
    DWARF_REG_MAP(16, rip), // return address
#if HAVE_XMM
    DWARF_FPU_REG(17, xmm_space[0]),
    DWARF_FPU_REG(18, xmm_space[4]),
    DWARF_FPU_REG(19, xmm_space[8]),
    DWARF_FPU_REG(20, xmm_space[12]),
    DWARF_FPU_REG(21, xmm_space[16]),
    DWARF_FPU_REG(22, xmm_space[20]),
    DWARF_FPU_REG(23, xmm_space[24]),
    DWARF_FPU_REG(24, xmm_space[28]),

    DWARF_FPU_REG(25, xmm_space[32]),
    DWARF_FPU_REG(26, xmm_space[36]),
    DWARF_FPU_REG(27, xmm_space[40]),
    DWARF_FPU_REG(28, xmm_space[44]),
    DWARF_FPU_REG(29, xmm_space[48]),
    DWARF_FPU_REG(30, xmm_space[52]),
    DWARF_FPU_REG(31, xmm_space[56]),
    DWARF_FPU_REG(32, xmm_space[60]),

    DWARF_FPU_REG(33, st_space[0]),
    DWARF_FPU_REG(34, st_space[4]),
    DWARF_FPU_REG(35, st_space[8]),
    DWARF_FPU_REG(36, st_space[12]),
    DWARF_FPU_REG(37, st_space[16]),
    DWARF_FPU_REG(38, st_space[20]),
    DWARF_FPU_REG(39, st_space[24]),
    DWARF_FPU_REG(40, st_space[28]),

    // todo: is this MM0-7 section correct?
    DWARF_FPU_REG(41, xmm_space[0]),
    DWARF_FPU_REG(42, xmm_space[2]),
    DWARF_FPU_REG(43, xmm_space[4]),
    DWARF_FPU_REG(44, xmm_space[6]),
    DWARF_FPU_REG(45, xmm_space[8]),
    DWARF_FPU_REG(46, xmm_space[10]),
    DWARF_FPU_REG(47, xmm_space[12]),
    DWARF_FPU_REG(48, xmm_space[14]),
#endif // HAVE_XMM
    DWARF_REG_MAP(49, eflags),

    DWARF_REG_MAP(50, es),
    DWARF_REG_MAP(51, cs),
    DWARF_REG_MAP(52, ss),
    DWARF_REG_MAP(53, ds),
    DWARF_REG_MAP(54, fs),
    DWARF_REG_MAP(55, gs),
    //
    // a few more regs omitted
    //
};


/**
 * AMD64 running 32-bit mode app
 */
static RegInfo regMap32[] =
{
    DWARF_REG_MAP(0, rax),
    DWARF_REG_MAP(1, rcx),
    DWARF_REG_MAP(2, rdx),
    DWARF_REG_MAP(3, rbx),
    DWARF_REG_MAP(4, rsp),
    DWARF_REG_MAP(5, rbp),
    DWARF_REG_MAP(6, rsi),
    DWARF_REG_MAP(7, rdi),
    DWARF_REG_MAP(8, rip),
    DWARF_REG_MAP(9, eflags),
};

static const size_t MAP_SIZE_32 = sizeof(regMap32)/sizeof(regMap32[0]);

#endif // (__x86_64__)

#undef REG_OFFSET
#undef DWARF_REG_MAP

static const size_t MAP_SIZE = sizeof(regMap)/sizeof(regMap[0]);


template<int> struct X86;


template<> struct X86<32>
{
    static inline size_t user_reg_pc() { return 8; }

    static inline size_t user_reg_fp() { return 5; } // ebp

    static inline size_t user_reg_sp() { return 4; } // esp

    static inline int user_reg_index(size_t n)
    {
        if (n < MAP_SIZE_32)
        {
            return regMap32[n].r_user;
        }
        Dwarf::ABI::out_of_range(__func__, n);
        return -1;
    }

    static inline const char* user_reg_name(size_t n)
    {
        if (n < MAP_SIZE_32)
        {
            return regMap32[n].r_name;
        }
        Dwarf::ABI::out_of_range(__func__, n);
        return "???";
    }

    static inline size_t user_reg_count() { return MAP_SIZE_32; }
};


template<> struct X86<64>
{
    static inline size_t user_reg_pc() { return 16; }

    static inline size_t user_reg_fp() { return 6; } // rbp

    static inline size_t user_reg_sp() { return 7; } // rsp

    static inline int user_reg_index(size_t n)
    {
        if (n < MAP_SIZE)
        {
            return regMap[n].r_user;
        }
        Dwarf::ABI::out_of_range(__func__, n);
        return -1;
    }

    static inline const char* user_reg_name(size_t n)
    {
        if (n < MAP_SIZE)
        {
            return regMap[n].r_name;
        }
        Dwarf::ABI::out_of_range(__func__, n);
        return "???";
    }

    static inline size_t user_reg_count() { return MAP_SIZE; }
};


template<int = 0> struct X86
{
    static inline size_t user_reg_pc()
    {
        if (Dwarf::get_addr_operations()->is_32_bit())
        {
            return X86<32>::user_reg_pc();
        }
        return X86<__WORDSIZE>::user_reg_pc();
    }

    static inline size_t user_reg_fp()
    {
        if (Dwarf::get_addr_operations()->is_32_bit())
        {
            return X86<32>::user_reg_fp();
        }
        return X86<__WORDSIZE>::user_reg_fp();
    }

    static inline size_t user_reg_sp()
    {
        if (Dwarf::get_addr_operations()->is_32_bit())
        {
            return X86<32>::user_reg_sp();
        }
        return X86<__WORDSIZE>::user_reg_sp();
    }

    static int inline user_reg_index(size_t n)
    {
        if (Dwarf::get_addr_operations()->is_32_bit())
        {
            return X86<32>::user_reg_index(n);
        }
        return X86<__WORDSIZE>::user_reg_index(n);
    }

    static inline const char* user_reg_name(size_t n)
    {
        if (Dwarf::get_addr_operations()->is_32_bit())
        {
            return X86<32>::user_reg_name(n);
        }
        return X86<__WORDSIZE>::user_reg_name(n);
    }

    static inline size_t user_reg_count()
    {
        if (Dwarf::get_addr_operations()->is_32_bit())
        {
            return X86<32>::user_reg_count();
        }
        return X86<__WORDSIZE>::user_reg_count();
    }
};


#endif // ABI_X86_H__8EE3A891_5B3E_4F34_9EE9_34ECEDF5036F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
