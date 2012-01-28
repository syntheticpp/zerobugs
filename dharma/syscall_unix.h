#ifndef SYSCALL_UNIX_H__7E32D6FB_BB28_4A1B_91AB_2F6886DD2CC4
#define SYSCALL_UNIX_H__7E32D6FB_BB28_4A1B_91AB_2F6886DD2CC4
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
#include "zdk/config.h"

namespace sys
{
    /**
     * Uses the ptrace wrapper to read the registers from the user area.
     */
    void get_regs(pid_t, user_regs_struct&);

    bool get_regs(pid_t, user_regs_struct&, std::nothrow_t) throw();

    void get_fpxregs(pid_t, user_fpxregs_struct&);

    void get_fpregs(pid_t, user_fpregs_struct&);

    void inline get_regs(pid_t lwpid, user_fpxregs_struct& regs)
    {
        get_fpxregs(lwpid, regs);
    }

    void set_fpxregs(pid_t, user_fpxregs_struct&);
    void inline set_regs(pid_t lwpid, user_fpxregs_struct& regs)
    {
        set_fpxregs(lwpid, regs);
    }
}

#endif // SYSCALL_UNIX_H__7E32D6FB_BB28_4A1B_91AB_2F6886DD2CC4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
