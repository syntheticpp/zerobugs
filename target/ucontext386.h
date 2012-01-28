#ifndef UCONTEXT386_H__968F684F_76F2_454B_B675_4A1EBADB05AE
#define UCONTEXT386_H__968F684F_76F2_454B_B675_4A1EBADB05AE
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

#include "zdk/export.h"


struct __attribute__((packed)) stack32_t
{
    uint32_t ss_sp;
    int32_t  ss_flags;
    uint32_t ss_size;
};

typedef uint32_t gregset32_t[19];

struct mcontext32_t
{
    gregset32_t gregs;
    // some stuff omitted
};
struct __attribute__((packed)) ucontext32_t
{
    uint32_t        uc_flags;
    uint32_t        uc_link;
    stack32_t       uc_stack;
    mcontext32_t    uc_mcontext;
    // some stuff omitted
};



inline void ZDK_LOCAL
read_signal_handler_frame_32(const Thread& thread, FrameImpl& f)
{
    ucontext32_t uc;

    bool gotRegs = false;
    try // sa_sigaction style handler?
    {
        addr_t ucptr = 0;
        thread_read(thread, f.stack_pointer() + 8, ucptr);
        thread_read(thread, ucptr, uc);

        gotRegs = uc.uc_mcontext.gregs[14] != 0; // EIP != 0
    }
    catch (...)
    {
    }
    if (!gotRegs)
    {
        // old-style signal handler?

        gregset32_t& gregs = uc.uc_mcontext.gregs;
        thread_read(thread, f.stack_pointer() + 4, gregs);
    }

    f.set_program_count(uc.uc_mcontext.gregs[14]);
    f.set_stack_pointer(uc.uc_mcontext.gregs[7]);
    f.set_frame_pointer(uc.uc_mcontext.gregs[6]);
}

#endif // UCONTEXT386_H__968F684F_76F2_454B_B675_4A1EBADB05AE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
