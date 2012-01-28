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

#include <iostream>
#include <mach/mach.h>
#include "dharma/syscall_wrap.h"
#include "dharma/system_error.h"

using namespace std;


void sys::get_regs(thread_act_t thread, user_regs_struct& regs)
{
    mach_msg_type_number_t count = user_regs_struct::count;
    int err = thread_get_state( thread,
                                user_regs_struct::flavor,
                                reinterpret_cast<thread_state_t>(&regs),
                                &count);
    if (err != KERN_SUCCESS)
    {
    #if DEBUG
        clog << __func__ << ": err=" << err << endl;
    #endif
        // todo: how do error codes translate into string messages?
        throw SystemError(err);
    }
}


void sys::get_fpxregs(thread_act_t thread, user_fpxregs_struct& regs)
{
     //
     // todo
     //
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
