//
// $Id: stack-i386.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <deque>
#include <iostream>
#include <vector>
#include <signal.h> // needed before ucontext.h on RH 7.2
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "cprologue.h"
#include "dharma/environ.h"
#include "stack_trace.h"
#include "target/trampoline.h"

using namespace std;


/**
 * Detect if we just entered a function, but not executed the
 * prologue code. Push a frame onto the stack if prologue detected.
 */
bool StackTraceImpl::check_prologue(const Thread& thread, const Frame& frame)
{
    return has_c_prologue32(thread, frame);
}


bool StackTraceImpl::unwind_frame(const Thread& thread, FrameImpl& f)
{
    if (f.frame_pointer() == 0)
    {
        return false; // we're done
    }

    if (check_trampoline_frame32(thread, f))
    {
        return true;
    }

    const reg_t sp = f.frame_pointer() + sizeof(reg_t);

    // sp now points to the saved instruction pointer %eip
    // sp + sizeof(reg_t) --> saved %ebp
    f.set_stack_pointer(sp + sizeof(reg_t));

    reg_t reg = 0;
    try
    {
        thread_read(thread, sp, reg);
        set_program_count(f, reg);

        thread_read(thread, f.frame_pointer(), reg);

        f.set_frame_pointer(reg);
    }
    catch (const exception&)
    {
        return false;
    }
    return true;
}


/*
Algorithm for Recovering A Corrupted Stack Trace in a Debugger
--------------------------------------------------------------

NOTES (Wed Aug 29 23:26:58 PDT 2007)
This document describes the behavior when ZERO_STACK_RECOVERY is set in the
environment.
I stopped working on this feature more than a couple of years ago (as soon as
I started adding support for reading frame unwinding information via
libdwarf).


A debugger is a tool used by programmers to examine the state of a program (debuggee).

An essential view of the debugged program's state is the stack trace.
A stack trace is essentialy a list of addresses, that the debugged program saved in a
special memory area called "stack".

The stack trace shows a succession of function calls that brought the debugged program to
its current state.

When the computer executes a program, it executes machine instructions read from memory, in sequence.
When an instruction that indicates that a function (or procedure) needs to be executed, the
computer saves the address of the next instruction in the sequence on the stack. Then, it executes
a "jump" to the address of the procedure. This "jump" disrupts the "normal" sequence of instructions.

Program in memory
+-----------------+
| instruction N   | start
+-----------------+
| N + 1           |
+-----------------+
| N + 2           |
+-----------------+
| ...             |
+-----------------+
| N + n           |
+-----------------+   call M
| N + n + 1       +-------------+
| call func. at M |             |
+-----------------+             |
| N + n + 2       |<----+       |
+-----------------+     |       |
| ...             |     |       |
+-----------------+     |       |
| M               |<----|-------+
+-----------------+     |
| ...             |     | address from stack: N + n + 2
+-----------------+     |
| M + n (return)  +-----+
+-----------------+
| ...             |

The figure shows a program that encounters a CALL instruction (at instruction N + n + 1).
The execution will "jump" at address M, where the called function begins;
but first, the address N + n + 2 is saved in the memory area called "stack", so that, when
the function "returns" (finishes), the sequence of instructions can be resumed from where
the program left.

Bugs sometimes destroy part of the current stack frame. For example, writing past the end of
an array stored on the stack (local array) can wipe out the return address. Because the normal
stack-tracing algorithm starts with the current frame, damage to this frame can prevent from showing
any of the earlier frames (it is as if a 'next' pointer in a linked list is corrupted).

When the stack-tracing algorithm fails, the debugger can often find the undamaged earlier frames
by starting at the bottom of the stack (assuming the stack grows downward) and working toward
the top of the stack (usually pointed-to by the current stack-pointer register).

My algorithm is an extension of the algorithm described in [1]. It works by searching for
saved frame pointers and assumes a stack layout where the saved address is stored immediately
after the frame pointer.

The key problem that my algorithm solves is that sometimes, addresses that match the frame
pointer that we're looking for are found on the stack, yet the location on the stack where
the match occurres is not a valid frame. Such a red-herring would throw the entire stack
reconstruction effort on the wrong track (again, it's like encountering an incorrect 'next'
pointer in the middle of traversing a linked list).

My algorithm extends the algorithm in [1] with a back-tracking approach. Every potential match
is memorized. The location (index in the stack) where the match occurred is saved into an
internal queue.

If the search reaches the top of the stack, and the last found frame pointer is
not equal to the value in the current frame pointer register (EBP on the i386), and not in an
EPSILON vicinity of the current stack pointer (ESP on the i386)(1), then a red-herring (spurious
frame pointer match) occurred. To recover from this, the last match that the algorithm saved
is thrown away, and the search resumes from that saved stack index. If the top of the stack is
again reached and the last found frame pointer does still not satisfy condition (1), then another
saved match is removed from the internal queue and we try again. This continues until either (1)
is satisfied or the internal queue is depleted.



REFERENCES
[1] Jonathan B. Rosenberg, How Debuggers Work. Algorithms, Data Structures, and Architecture.
Wiley, 1996 pag. 140
*/

/*
struct FrameInfo // for stack recovery with backtracking
{
    addr_t fp;
    size_t pos;
    addr_t pc;
    addr_t sp;
};


static word_t backtrack
(
    const vector<word_t>& stk,
    addr_t sp,
    addr_t ss,
    word_t fp,
    size_t startPos,
    deque<FrameInfo>& trace
)
{
    assert(startPos <= stk.size());

    word_t result = 0;

    for (size_t i = startPos; i; --i)
    {
    #ifdef DEBUG
        clog << __func__ << " [" << sp + sizeof(word_t) * (i - 1);
        clog << "]: " << stk[i - 1] << endl;
    #endif
        for (size_t j = i; j; --j)
        {
            if (stk[j - 1] == fp)
            {
                addr_t pc = stk[j];

                FrameInfo f;
                f.pc = pc;
                f.fp = fp;
                f.sp = sp + sizeof(word_t) * j;
                f.pos = i - 1;

                trace.push_back(f);
                fp = f.sp - sizeof(word_t);

                i = j;
                result = fp;
                break;
            }
        }
    }
    return result;
}



bool StackTraceImpl::verify_recover(const Thread& thread, const Frame& f)
{
    static const bool recover = env::get_bool("ZERO_STACK_RECOVERY");

    if (recover && thread.is_live()
        // Stack grows downwards on the x86 -- if this
        // happens, it is very likely that the stack is
        // corrupted:
        && (f.stack_pointer() < thread.stack_pointer()))
    {
    #ifdef DEBUG
        clog << __func__ << ": SP=" << (void*)f.stack_pointer() << endl;
    #endif
        recover_corrupted(thread);
        complete_ = true;
        return true;
    }
    return false;
}
*/


/**
 * Work the stack from bottom to top
 */
/*
void StackTraceImpl::recover_corrupted(const Thread& thread)
{
    assert(thread.is_live());

    deque<FrameInfo> trace;

    const addr_t ss = thread.stack_start();
    const addr_t sp = thread.stack_pointer();
    const word_t bp = thread.frame_pointer();

    if (ss >= sp)
    {
        // stack must be word-aligned
        assert((ss - sp) % sizeof(word_t) == 0);

        const size_t size = (ss - sp) / sizeof(word_t);

        vector<word_t> stk(size);
        thread.read_data(sp, &stk[0], size);

        size_t pos = size;
        for (word_t fp = 0;;)
        {
            word_t res = backtrack(stk, sp, ss, fp, pos, trace);
            if (res && (res == bp))
            {
                break;
            }
            if (trace.empty())
            {
                break;
            }
            static const int diff = env::get("ZERO_STACK_DIFF", 16);
            if (diff && abs(static_cast<long>(res - sp)) <= diff)
            {
                break;
            }
            FrameInfo f = trace.back();

            fp = f.fp;
            pos = f.pos;

            trace.pop_back();
        }
    }

    if (!trace.empty())
    {
#ifdef DEBUG
        clog << __func__ << ": reconstructing stack trace\n";
#endif
        FrameList::iterator first = frames_.begin();
        if (first != frames_.end())
        {
            ++first;
        }
        frames_.erase(first, frames_.end());

        FrameImpl frame(*this, thread);

        deque<FrameInfo>::reverse_iterator i = trace.rbegin();
        for (; i != trace.rend(); ++i)
        {
    #ifdef DEBUG
            clog << '\t' << hex << i->pc << dec << endl;
    #endif
            frame.set_program_count(i->pc);
            frame.set_stack_pointer(i->sp);
            frame.set_frame_pointer(i->sp - sizeof(word_t));

            frames_.push_back(new FrameImpl(frame, frames_.size()));
        }
    }
}
*/


bool StackTraceImpl::unwind_begin(const Thread& t, FrameImpl& f, size_t depth)
{
    if (frames_.size() + 1 >= depth)
    {
        return false;
    }
    frames_.push_back(new FrameImpl(f, frames_.size()));

    return true;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
