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

#include "zdk/thread_util.h"
#include "cprologue.h"
#include "stack_trace.h"
using namespace std;


bool has_c_prologue64(const Thread& thread, const Frame& f)
{
    const addr_t pc = f.program_count();

    word_t word = 0;
    size_t count = 0;

    thread_read(thread, pc, word, &count);

    // 55               push   %rbp
    // 48 89 e5         mov    %rsp,%rbp

    if ((word & 0xffffffff) == 0xe5894855
     || (word & 0xffffffff) == 0xe58948cc   // breakpoint?
     || (word & 0x00ffffff) == 0x00ec8348)  // sub N, %rsp
    {
        // debuggee interrupted while in prologue code?
        if (f.program_count() >= pc + 4)
        {
            return true;                    // have full frame
        }
        else if (f.program_count() == pc)
        {
            return true;
        }
    }
    return false;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
