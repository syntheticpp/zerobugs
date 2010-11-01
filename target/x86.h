#ifndef X86_H__3B0CD22B_0C2F_11DC_B8A3_00C04F09BBCC
#define X86_H__3B0CD22B_0C2F_11DC_B8A3_00C04F09BBCC
//
// $Id: x86.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/assert.h"
#include "zdk/config.h"
#include "zdk/export.h"
#include "zdk/platform.h"

using Platform::addr_t;
using Platform::word_t;


inline word_t ZDK_LOCAL
x86_get_breakpoint_opcode(word_t word)
{
    static const word_t x86_BREAK_OPCODE = 0xCC;
    union
    {
        char byte[sizeof(word_t)];
        word_t word;
        addr_t addr;
    } x;

    x.word = word;
    x.byte[0] = x86_BREAK_OPCODE;

#ifdef __i386__
    assert (((word & 0xFFFFFF00) | x86_BREAK_OPCODE) == x.addr);
#endif
    return x.word;
}

#endif // X86_H__3B0CD22B_0C2F_11DC_B8A3_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
