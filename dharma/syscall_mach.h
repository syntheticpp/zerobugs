#ifndef SYSCALL_MACH_H__EE791FB9_9E3B_4371_AFF7_126B9DEB038E
#define SYSCALL_MACH_H__EE791FB9_9E3B_4371_AFF7_126B9DEB038E
//
// $Id: syscall_mach.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <mach/mach_types.h>

namespace sys
{
    void get_regs(thread_act_t, user_regs_struct&);

    bool get_regs(thread_act_t, user_regs_struct&, std::nothrow_t) throw();

    void get_fpxregs(thread_act_t, user_fpxregs_struct&);

    void get_fpregs(thread_act_t, user_fpregs_struct&);

    void inline get_regs(thread_act_t thread, user_fpxregs_struct& fpxregs)
    {
        get_fpxregs(thread, fpxregs);
    }
}
#endif // SYSCALL_MACH_H__EE791FB9_9E3B_4371_AFF7_126B9DEB038E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
