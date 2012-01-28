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
#include "zdk/export.h"
#include "zdk/thread_util.h"
#include "target/trampoline.h"
#include "stack_trace.h"

using namespace std;


/**
 * Check for C function prologue at given code address
 */
static bool
have_cprologue(const Thread& thread, addr_t addr, word_t& frameOffs)
{
    word_t w = 0;
    size_t n = 0;

    thread.read_code(addr, &w, 1, &n);

    if ((w & 0xffff0000) == 0x94210000) // STWU R1, ...
    {
        frameOffs = int16_t(w & 0xffff);

        return true;
    }
    return false;
}


/**
 * For a given frame, find the saved value of R1
 */
static addr_t
get_stack_ptr(const Thread& thread,
              const Frame& frame,
              bool computeFramePtr = false)
{
    addr_t ptr = frame.stack_pointer();

    if (SymbolMap* symbols = thread.symbols())
    {
        const addr_t pc = frame.program_count();

        if (RefPtr<Symbol> sym = symbols->lookup_symbol(pc))
        {
            const addr_t funStartAddr = sym->addr() - sym->offset();

            if (funStartAddr < pc)
            {
                word_t offset = 0;

                if (have_cprologue(thread, funStartAddr, offset))
                {
                    if (computeFramePtr)
                    {
                        ptr += offset;
                    }
                    else
                    {
                        ptr -= offset;
                    }
                }
            }
        }
    }

    return ptr;
}


bool
StackTraceImpl::check_prologue(const Thread& thread, const Frame& frame)
{
    word_t offset; // ignored
    return have_cprologue(thread, frame.program_count(), offset);
}


struct ZDK_LOCAL Link
{
    addr_t  prev; // saved Link Register, points to previous link
    reg_t   retAddr;
};


bool
StackTraceImpl::unwind_frame(const Thread& thread, FrameImpl& frame)
{
    // the value of the link register saved in the signal frame (if detected)
    addr_t signalLinkReg = 0;

    if (signalFrame_) // the frame before the signal
    {
        if (frame.skip())
        {
            signalLinkReg = signalFrame_->real_program_count();
            signalFrame_->set_real_program_count(signalFrame_->program_count());

            frame.set_skip(false);

            frames_.push_back(signalFrame_);
            signalFrame_.reset();
        }
        else
        {
            frame.set_skip(true);
            frame.set_signal_handler(true);
            return true;
        }
    } // signal frame

    if (frame.frame_pointer() == 0)
    {
        return false; // we're done
    }
    try
    {
        const addr_t backLinkAddr = frame.stack_pointer();

        Link link = { 0 };

        thread_read(thread, backLinkAddr, link);

        if (link.prev)
        {
            if (signalLinkReg)
            {
                link.retAddr = signalLinkReg;
            }
            //set program counter to saved return address
            frame.set_program_count(link.retAddr);

            RefPtr<FrameImpl> tmp = new FrameImpl(frame, frames_.size());

            if (check_trampoline_frame32(thread, *tmp))
            {
                signalFrame_ = tmp;
                frame.set_stack_pointer(tmp->stack_pointer());
            }
            else
            {
                frame.set_stack_pointer(link.prev);
            }
            const reg_t fp = get_stack_ptr(thread, frame, true);
            frame.set_frame_pointer(fp);

            if (fp == frame.stack_pointer())
            {
                // if the frame pointer and stack pointer are at the same
                // address, then the return address was not saved in the
                // link area
                if (frames_.size() <= 1)
                {
                    // and if we're at the top of the stack, then
                    // read it from the Link Register
                    frame.set_program_count(thread.read_register(PT_LNK, false));
                }
            }
        }
        else // end of stack
        {
            frame.set_frame_pointer(0);
            frame.set_program_count(0);
        }
        return true;
    }
    catch (const exception& e)
    {
#ifdef DEBUG
        clog << __func__ << ": " << e.what() << endl;
#endif
    }
    return false;
}


bool
StackTraceImpl::unwind_begin(const Thread& thread, FrameImpl& frame, size_t depth)
{
    if (frames_.size() + 1 >= depth)
    {
        return false;
    }
    frame.set_stack_pointer(get_stack_ptr(thread, frame));

    RefPtr<FrameImpl> top = new FrameImpl(frame, frames_.size());
    frames_.push_back(top);

    return true;
}

/*
bool
StackTraceImpl::verify_recover(const Thread&, const Frame&)
{
    return false;
}
*/
