#ifndef ABI_PPC_H__301E693F_3858_4F83_AE31_05CAB966DC34
#define ABI_PPC_H__301E693F_3858_4F83_AE31_05CAB966DC34
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
#include <asm/ptrace.h>

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


// place holder: revisit later
struct Arch
{
    static inline size_t user_reg_pc() { return PT_NIP; }

    static inline size_t user_reg_fp() { return PT_R31; }

    static inline size_t user_reg_sp() { return PT_R1; }

    static inline int user_reg_index(size_t n)
    {
        return n;
    }

    static inline const char* user_reg_name(size_t n)
    {
        return "???";
    }

    static inline size_t user_reg_count() { return 48 /* ELF_NGREG */; }
};
#endif // ABI_PPC_H__301E693F_3858_4F83_AE31_05CAB966DC34
