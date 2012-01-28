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

#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "cprologue.h"
#include "stack_trace.h"
#include "target/trampoline.h"

#ifdef DEBUG
 #include <iostream>
 using namespace std;
#endif


bool
StackTraceImpl::check_prologue(const Thread& thread, const Frame& frame)
{
    if (thread.is_32_bit())
    {
        return has_c_prologue32(thread, frame);
    }
    else
    {
        return has_c_prologue64(thread, frame);
    }
}


bool StackTraceImpl::unwind_frame(const Thread& thread, FrameImpl& f)
{
    if (f.frame_pointer() == 0)
    {
        return false;
    }

    reg_t reg = 0;
    size_t wordsRead = 0;

    unsigned long mask = 0xffffffffffffffffULL;
    size_t wordSize = sizeof(word_t);

    if (thread.is_32_bit())
    {
        wordSize = 4;
        mask = 0x0ffffffffULL;
    }

    if (check_trampoline_frame64(thread, f))
    {
        return true;
    }

    if (thread.is_syscall_pending(f.program_count()))
    {
        const addr_t sp = f.stack_pointer();

        if (!thread_read(thread, sp + wordSize, reg, &wordsRead))
        {
            return false;
        }

        reg &= mask;
        set_program_count(f, reg);

        if (!thread_read(thread, sp, reg, &wordsRead))
        {
            return false;
        }
        f.set_frame_pointer(reg & mask);
        f.set_stack_pointer(f.stack_pointer() + 2 * wordSize);

        return true;
    }

    if (!thread_read(thread, f.frame_pointer() + wordSize, reg, &wordsRead))
    {
        return false;
    }
    reg &= mask;

/**** breaks recursive functions, can't do
    if (reg == f.program_count())
    {
        return false;
    }
 *****/
    set_program_count(f, reg);
    assert(f.real_program_count() == f.program_count());

    if (!thread_read(thread, f.frame_pointer(), reg, &wordsRead))
    {
        return false;
    }

    reg &= mask;

    f.set_stack_pointer(f.frame_pointer() + 2 * wordSize);
    f.set_frame_pointer(reg);

    assert(f.frame_pointer() == reg);

    return true;
}


bool
StackTraceImpl::unwind_begin(const Thread& t, FrameImpl& f, size_t depth)
{
    frames_.push_back(RefPtr<FrameImpl>(new FrameImpl(f, frames_.size())));
    return frames_.size() < depth;
}

/*
void StackTraceImpl::recover_corrupted(const Thread&)
{
}


bool StackTraceImpl::verify_recover(const Thread&, const Frame&)
{
    return false;
}
*/

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
