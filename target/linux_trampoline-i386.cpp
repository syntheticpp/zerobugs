//
// $Id: linux_trampoline-i386.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#ifndef __linux__
 #error bummer
#endif

#include "engine/stack_trace.h"
#include "engine/thread.h"
#include "zdk/thread_util.h"
#include "ucontext386.h"
#include "trampoline.h"

using namespace std;


static bool in_trampoline32(const Thread& thread, addr_t pc)
{
    size_t count = 0;

    uint32_t w[2] = { 0, 0 };

    thread_read(thread, pc, w, &count);

    /* check for the following code (kernel 2.4 - 2.6):
        58              POP EAX
        b8 77 00 00 00  MOX EAX, 0x77
        cd 80           INT 0x80

    or:
        b8 ad 00 00 00  MOV EAX, 0xad
        cd 80           INT 0x80

    */

    if ((w[0] == 0x0077b858 && w[1] == 0x80cd0000) ||
        (w[0] == 0xadb8 && (w[1] & 0xffffff) == 0x80cd00))
    {
        return true;
    }

    return false;
}


bool check_trampoline32(const Thread& thread, const Frame& f)
{
    const addr_t pc = f.program_count(); // program counter at prev frame

    return in_trampoline32(thread, pc)
        || in_trampoline32(thread, program_count(thread));
}


bool
check_trampoline_frame32(const Thread& thread, FrameImpl& f)
{
    if (check_trampoline32(thread, f))
    {
        read_signal_handler_frame_32(thread, f);

        f.set_signal_handler();
        return true;
    }
    return false;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
