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

#include "zdk/config.h"
#include "zdk/xtrace.h"
#include "syscall_wrap.h"
#include <new> // for nothrow_t

using namespace std;


bool
sys::get_regs(pid_t pid, user_regs_struct& regs, nothrow_t) throw()
{
    if (XTrace::ptrace(static_cast<__ptrace_request>(PTRACE_GETREGS),
                       pid,
                       GETREGS_NATIVE(&regs)
                      ) == 0)
    {
        return true;
    }
    return false;
}


void sys::get_regs(pid_t pid, user_regs_struct& regs)
{
    sys::ptrace(
        static_cast<__ptrace_request>(PTRACE_GETREGS),
        pid,
        GETREGS(&regs));
}


void sys::get_fpregs(pid_t pid, user_fpregs_struct& regs)
{
    sys::ptrace(
        static_cast<__ptrace_request>(PTRACE_GETFPREGS),
        pid,
        GETREGS(&regs));
}


void sys::get_fpxregs(pid_t pid, user_fpxregs_struct& regs)
{
    static const __ptrace_request req =

#ifdef HAVE_STRUCT_USER_FPXREGS_STRUCT
        static_cast<__ptrace_request>(PTRACE_GETFPXREGS);
#else
        static_cast<__ptrace_request>(PTRACE_GETFPREGS);
#endif
    sys::ptrace(req, pid, GETREGS(&regs));
}


void sys::set_fpxregs(pid_t pid, user_fpxregs_struct& regs)
{
    static const __ptrace_request req =

#ifdef HAVE_STRUCT_USER_FPXREGS_STRUCT
        static_cast<__ptrace_request>(PTRACE_SETFPXREGS);
#else
        static_cast<__ptrace_request>(PTRACE_SETFPREGS);
#endif
    sys::ptrace(req, pid, GETREGS(&regs));
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
