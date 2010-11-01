// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: linux_trampoline-ppc.cpp 714 2010-10-17 10:03:52Z root $
//
#include "zdk/zero.h"
#include "zdk/thread_util.h"
#include "engine/stack_trace.h"
#include "frame_reg.h"
#include "trampoline.h"


using namespace std;
using namespace Platform;


static inline void
set_frame_reg(const Thread& thread,
              FrameImpl& frame,
              size_t n,
              const gregset_t& gregs)
{
    char name[32];
    snprintf(name, 32, "r%d", n);

    RefPtr<Register> r = user_reg<word_t>(thread, name, n * sizeof(reg_t), gregs[n]);
    frame.set_user_object(name, r.get());
}


static inline void
set_frame_reg(const Thread& thread,
              FrameImpl& frame,
              const char* name,
              size_t n,
              const gregset_t& gregs)
{
    RefPtr<Register> r = user_reg<word_t>(thread, name, n * sizeof(reg_t), gregs[n]);
    frame.set_user_object(name, r.get());
}


bool check_trampoline_frame32(const Thread& thread, FrameImpl& frame)
{
    word_t w[2] = { 0, 0 };
    size_t n = 0;

    thread.read_code(frame.program_count(), &w[0], 2, &n);

    if (w[1] == 0x44000002 && (w[0] == 0x38000077 || w[0] == 0x380000ac))
    {
        addr_t savedRegAddr = 0;

        // initial offs: 64 (sig frame size) + 28 (offs of uc_regs in sigcontext)
        // works for a handler set with signal()
        //
        // 256: offset to reg ptr with sigaction (determined experimentally)
        for (size_t i = 0, offs = 64 + 28; i != 2; ++i, offs = 256)
        try
        {
            thread_read(thread, frame.stack_pointer() + offs, savedRegAddr);

            gregset_t gregs = { 0 };
            thread_read(thread, savedRegAddr, gregs);

            addr_t sp = gregs[PT_R1];

            if (sp == 0 || gregs[PT_NIP] == 0)
            {
                continue; // try sigaction frame
            }
            frame.set_program_count(gregs[PT_NIP]);
            frame.set_real_program_count(gregs[PT_LNK]);
            frame.set_frame_pointer(gregs[PT_R31]);

            thread_read(thread, sp, sp);
            frame.set_stack_pointer(sp);

            set_frame_reg(thread, frame, PT_R0, gregs);

            // 2nd small data area pointer
            set_frame_reg(thread, frame, PT_R2, gregs);
            // registers used for param passing and return values
            set_frame_reg(thread, frame, PT_R3, gregs);
            set_frame_reg(thread, frame, PT_R4, gregs);

            // registers for param passing
            for (size_t pt_rx = PT_R5; pt_rx <= PT_R10; ++pt_rx)
            {
                set_frame_reg(thread, frame, pt_rx, gregs);
            }

            // small data area base
            set_frame_reg(thread, frame, PT_R13, gregs);

            // Non-volatile registers used for local variables.
            for (size_t pt_rx = PT_R14; pt_rx <= PT_R31; ++pt_rx)
            {
                set_frame_reg(thread, frame, pt_rx, gregs);
            }
            set_frame_reg(thread, frame, "msr", PT_MSR, gregs);
            set_frame_reg(thread, frame, "ctr", PT_CTR, gregs);
            set_frame_reg(thread, frame, "ccr", PT_CCR, gregs);
            set_frame_reg(thread, frame, "xer", PT_XER, gregs);
            return true;
        }
        catch (...)
        {
        }
    }

    return false;
}

bool check_trampoline_frame64(const Thread&, FrameImpl&)
{
    return false;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
