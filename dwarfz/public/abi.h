#ifndef ABI_H__542DCFD8_65E6_4542_A85F_C10B5277A330
#define ABI_H__542DCFD8_65E6_4542_A85F_C10B5277A330
//
// $Id$
// DWARF register number mapping
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#if __i386__
 #include "abi_x86.h"
 typedef X86<32> Arch;

#elif __x86_64__
 #include "abi_x86.h"

 #if _XDEBUG_32_ON_64
    typedef X86<> Arch; // debug both 64 and 32bit apps
 #else
    typedef X86<64> Arch;
 #endif

#elif __PPC__
 #include "abi_ppc.h"

#endif


namespace Dwarf
{
    namespace ABI
    {
        /**
         * Given a dwarf register number, return its index
         * in the user_regs_struct.
         */
        inline int user_reg_index(size_t n)
        {
            return Arch::user_reg_index(n);
        }

        /**
         * Given a dwarf register number, return its name
         * as a string, or NULL if not found.
         */
        inline const char* user_reg_name(size_t n)
        {
            return Arch::user_reg_name(n);
        }

        inline size_t user_reg_count() { return Arch::user_reg_count(); }

        inline size_t user_reg_pc() { return Arch::user_reg_pc(); }

        inline size_t user_reg_fp() { return Arch::user_reg_fp(); }

        inline size_t user_reg_sp() { return Arch::user_reg_sp(); }
    }
}

#endif // ABI_H__542DCFD8_65E6_4542_A85F_C10B5277A330
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
