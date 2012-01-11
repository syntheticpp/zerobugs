//
// $Id: linux_trampoline-x86_64.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// NOTE: this may be deprecated now that there's stub_rt_sigreturn
#ifndef __linux__
 #error bummer
#endif

#include <iostream>
#include <sys/ucontext.h>
#include "zdk/thread_util.h"
#include "zdk/zero.h"
#include "trampoline.h"
#include "engine/stack_trace.h" // for FrameImpl full definition
#include "ucontext386.h"        // for 32bit ucontext



bool check_trampoline64(const Thread& thread, const Frame& f)
{
    // the program counter at the previous frame
    addr_t pc = f.program_count();

    word_t w[2] = { 0, 0 };
    size_t count = 0;

    thread.read_code(pc, w, 2, &count);

    /* check for the following code:

        mov $0xf,%rax   48 c7 c0 0f 00 00 00
        syscall         0f 05

        --- or ----

        mov $0xad,%eax  b8 ad 00 00 00
        syscall         0f 05

        mov $0x77,%eax  b8 77 00 00 00
        syscall         0f 05

        (The latter appear to apply to 32-bit apps)

    */

    bool result = false;

    if ((count == 2 &&
        (w[0] == 0x0f0000000fc0c748) && (w[1] & 0xff) == 0x05)
     || (count > 0 &&
            ((w[0] & 0xffffffffffffff) == 0x50f000000adb8
          || (w[0] & 0xffffffffffffff00) == 0x50f00000077b800)))
    {
        result = true;
    }
#if DEBUG
    std::clog << __func__ << ": " << result << std::endl;
#endif
    return result;
}



bool check_trampoline_frame64(const Thread& thread, FrameImpl& f)
{
    bool result = false;

    if (check_trampoline64(thread, f))
    {
        if (thread.is_32_bit())
        {
            read_signal_handler_frame_32(thread, f);
        }
        else
        {
            ucontext_t uc;
            thread_read(thread, f.stack_pointer(), uc);

            f.set_program_count(uc.uc_mcontext.gregs[REG_RIP]);
            f.set_stack_pointer(uc.uc_mcontext.gregs[REG_RSP]);
            f.set_frame_pointer(uc.uc_mcontext.gregs[REG_RBP]);
        }
        assert(f.real_program_count() == f.program_count());
        f.set_signal_handler();

        result = true;
    }
    return result;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
