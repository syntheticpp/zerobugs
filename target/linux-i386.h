#ifndef LINUX_386_H__13695992_6C6A_487D_9A7D_9DE8BBDC593E
#define LINUX_386_H__13695992_6C6A_487D_9A7D_9DE8BBDC593E
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
#if !defined(__i386__) && !defined(__x86_64__)
 #error File included incorrectly
#endif

#include "zdk/check_ptr.h"
#include "zdk/stack.h"

#ifdef __x86_64__
 #define EBP    4  // RBP
 #define EIP    16 // RIP
 #define UESP   19 // RSP
#endif


inline bool
get_frame_reg32(StackTrace* trace, size_t regIndex, reg_t& r)
{
    assert(trace);

    switch (regIndex)
    {
    case UESP:
        r = CHKPTR(trace->selection())->stack_pointer();
        return true;

    case EBP:
        r = CHKPTR(trace->selection())->frame_pointer();
        return true;

    case EIP:
        r = CHKPTR(trace->selection())->program_count();
        return true;
    }
    return false;
}

#endif // LINUX_386_H__13695992_6C6A_487D_9A7D_9DE8BBDC593E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
