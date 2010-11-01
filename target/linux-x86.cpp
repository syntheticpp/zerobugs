//
// $Id: linux-x86.cpp 720 2010-10-28 06:37:54Z root $
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
 #error incorrect platform
#endif

#include "zdk/check_ptr.h"
#include "dharma/syscall_wrap.h"
#include "debug_regs_386.h"
#include "linux_live.h"
#include "x86.h"

using namespace std;


word_t
LinuxLiveTarget::get_breakpoint_opcode(word_t word) const
{
    return x86_get_breakpoint_opcode(word);
}


ZObject* LinuxLiveTarget::regs(const Thread& thread) const
{
    ZObject* result = thread.regs();
    if (!result)
    {
        RefPtr<GRegs> r(new GRegs);
        sys::get_regs(thread.lwpid(), *r);
        result = thread.regs(r.get());
    }
    return result;
}


ZObject* LinuxLiveTarget::fpu_regs(const Thread& thread) const
{
    ZObject* result = thread.fpu_regs();
    if (!result)
    {
        RefPtr<FPXRegs> r(new FPXRegs);
        sys::get_fpxregs(thread.lwpid(), *r);

        result = thread.fpu_regs(r.get());
    }
    return result;
}



void
LinuxLiveTarget::step_until_safe(Thread& thread, addr_t addr) const
{
    if (thread.is_event_pending())
    {
        // no need to take this thread out of the dangerous area,
        // since it will not be resumed automatically
        return;
    }
    addr_t pc = thread.program_count();

    while ((pc > addr) && (pc - addr < sizeof(word_t)))
    {
        if (thread_finished(thread))
        {
            break;
        }
        const addr_t pcOld = pc;

        Runnable* runnable = CHKPTR(get_runnable(&thread));
        runnable->step_instruction();

        pc = thread.program_count();

        if (pc == pcOld)
        {
            // single-byte spinlock opcode? does the intel CPU
            // have such an instruction? and what if the thread
            // has been killed/interrupted/exited?
            fprintf(stderr, "%p Spinlock?\n", (void*)pc);
            break;
        }
    }
}


addr_t
LinuxLiveTarget::setup_caller_frame(Thread& thread, addr_t sp, long pc)
{
    // push the current program counter so that the called
    // function returns here
    Platform::dec_word_ptr(thread, sp);
    thread_poke_word(thread, sp, pc);

    CHKPTR(get_runnable(&thread))->set_stack_pointer(sp);

    return sp;
}



auto_ptr<DebugRegsBase>
LinuxLiveTarget::get_debug_regs(Thread& thread) const
{
    return auto_ptr<DebugRegsBase>(new DebugRegs386(thread));
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
