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

#include "zdk/zero.h"
#include "zdk/thread_util.h"
#include "cprologue.h"
#include "stack_trace.h"


/*
bool has_c_prologue32(const Thread& thread, FrameImpl& f)
{
    word_t word = 0;
    size_t wordsRead = 0;

    thread_read(thread, f.program_count(), word, &wordsRead);

    // gcc-compiled C functions typically start with this sequence:
    // 55                      push   %ebp
    // 89 e5                   mov    %esp,%ebp
    // 83 ec XX                sub    $0xXX,%esp

    if ((word & 0x00ffffff) == 0x00e58955
     || (word & 0x00ffffff) == 0x00ec8b55
     || (word & 0x00ffffff) == 0x00e589cc   // breakpoint?
     || (word & 0x00ffffff) == 0x00ec8bcc)
    {
        const reg_t stackPointer = f.stack_pointer();

        // we might have just entered a C function, the word
        // on the stack is very likely to be a return address

        thread_read(thread, stackPointer, word, &wordsRead);

        f.set_program_count(word);

        // %ebp is saved on the stack, so bump the stack ptr
        f.set_stack_pointer(stackPointer + sizeof (reg_t));

        return true;
    }

    return false;
}
*/


bool has_c_prologue32(const Thread& thread, const Frame& f)
{
    word_t word = 0;
    size_t wordsRead = 0;

    thread_read(thread, f.program_count(), word, &wordsRead);

    // gcc-compiled C functions typically start with this sequence:
    // 55                      push   %ebp
    // 89 e5                   mov    %esp,%ebp
    // 83 ec XX                sub    $0xXX,%esp

    if ((word & 0x00ffffff) == 0x00e58955
     || (word & 0x00ffffff) == 0x00ec8b55
     || (word & 0x00ffffff) == 0x00e589cc   // breakpoint?
     || (word & 0x00ffffff) == 0x00ec8bcc)
    {
        return true;
    }
    return false;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
