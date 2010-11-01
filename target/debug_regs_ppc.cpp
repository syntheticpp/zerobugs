//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: debug_regs_ppc.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// ************** Support for PowerPC debug regs ***************
//
#include "zdk/xtrace.h"
#include "zdk/zero.h"
#include "dharma/syscall_wrap.h"
#include "debug_regs_ppc.h"
#include "engine/ptrace.h" // for PT_GETSIGINFO
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <stdexcept>

using namespace std;


DebugRegsPPC::DebugRegsPPC(Thread& thread) : thread_(thread)
{
    memset(&watchAddr_[0], 0, sizeof watchAddr_);
}


size_t DebugRegsPPC::max_size(Condition cond) const
{
    switch (cond)
    {
    case BREAK_ON_ANY:
        return MAX_REG;

    case BREAK_ON_INSTRUCTION:
        return MAX_REG / 2;

    case BREAK_ON_DATA_WRITE:
    case BREAK_ON_DATA_READ_WRITE:
        return MAX_REG / 2;

    default:
        clog << __func__ << ": invalid argument " << (void*)cond << endl;
        // throw invalid_argument(__func__);
    }
    return 0;
}



bool DebugRegsPPC::set_breakpoint
(
    addr_t      addr,
    uint32_t*   index,
    bool        global,
    Condition   cond,
    Length      length
)
{
    if (global)
    {
        return false; // not supported
    }
    int n = 0;

    switch (cond)
    {
    case BREAK_ON_INSTRUCTION:
        if (!find_available_iabr(n))
        {
            return false;
        }
        break;

    case BREAK_ON_DATA_WRITE:
    case BREAK_ON_DATA_READ_WRITE:
        if (!find_available_dabr(n))
        {
            return false;
        }
        break;

    default:
        assert(false); // should never get here, due to max_size
                       // returning 0 for any conditions other
                       // BREAK_ON_INSTRUCTION and BREAK_ON_DATA_READ_WRITE
        return false;
    }

    if (XTrace::ptrace(__ptrace_request(PTRACE_SET_DEBUGREG),
                       thread_.lwpid(), n, addr) < 0)
    {
    #if DEBUG
        clog << "PTRACE_SET_DEBUGREG failed, n=" << n << " error=" << errno << endl;
    #endif
        return false;
    }
#if DEBUG
    else
    {
        clog << "PTRACE_SET_DEBUGREG ok, n=" << n << endl;
    }
#endif

    watchAddr_[n] = addr;
    if (index)
    {
        *index = n;
    }
    return true;
}



void DebugRegsPPC::clear(reg_t n)
{
    if (n >= MAX_REG)
        throw invalid_argument("DebugRegsPPC: register out of range");

    if (watchAddr_[n])
    {
        sys::ptrace(__ptrace_request(PTRACE_SET_DEBUGREG), thread_.lwpid(), n, 0);
        watchAddr_[n] = 0;
    }
}


void DebugRegsPPC::enable
(
    uint32_t    n,
    bool        activate,
    bool     /* global */,
    Condition   cond,
    Length   /* ignored */
)
{
    if (n >= MAX_REG)
        throw invalid_argument("DebugRegsPPC: register out of range");

    if (!activate)
    {
        if (watchAddr_[n])
        {
            sys::ptrace(__ptrace_request(PTRACE_SET_DEBUGREG), thread_.lwpid(), n, 0);
        }
    }
    else
    {
        switch (cond)
        {
        case BREAK_ON_INSTRUCTION:
            if (n < MAX_REG / 2)
            {
                throw invalid_argument("DebugRegsPPC: IABR condition mismatch");
            }
            break;

        case BREAK_ON_DATA_WRITE:
        case BREAK_ON_DATA_READ_WRITE:
            if (n > MAX_REG / 2)
            {
                throw invalid_argument("DebugRegsPPC: DABR condition mismatch");
            }
            break;

        default:
            throw invalid_argument("DebugRegsPPC: unsupported condition");
        }
        addr_t addr = watchAddr_[n];
        assert(addr);

        sys::ptrace(__ptrace_request(PTRACE_SET_DEBUGREG), thread_.lwpid(), n, addr);
    }
}


addr_t DebugRegsPPC::addr_in_reg(reg_t n) const
{
    if (n >= MAX_REG)
        throw invalid_argument("DebugRegsPPC: register out of range");

#if 0
    return watchAddr_[n];
#else
    addr_t addr = 0;

    XTrace::ptrace(__ptrace_request(PTRACE_GET_DEBUGREG), thread_.lwpid(), n, addr_t(&addr));
    return addr;
#endif
}


addr_t DebugRegsPPC::hit(reg_t* condition)
{
    addr_t result = 0;
    siginfo_t siginfo = { 0 };

    if (XTrace::ptrace(PT_GETSIGINFO, thread_.lwpid(), 0, (addr_t)&siginfo) == 0)
    {
        if (siginfo.si_signo == SIGTRAP)
        {
            // is the address where the TRAP occurred amongst
            // the addresses that we are watching for?
            for (unsigned i = 0; i != MAX_REG; ++i)
            {
                if (watchAddr_[i] == addr_t(siginfo.si_addr))
                {
                    result = watchAddr_[i];

                    if (condition)
                    {
                        if (i < MAX_REG / 2)
                        {
                            // DABR
                            *condition = BREAK_ON_DATA_READ_WRITE;
                        }
                        else
                        {
                            // IABR
                            *condition = BREAK_ON_INSTRUCTION;
                        }
                    }
                    break;
                }
            }
        }
    }
    return result;
}


void DebugRegsPPC::dump(std::ostream& outs) const
{
}



bool DebugRegsPPC::find_available_dabr(int& n) const
{
    for (unsigned i = 0; i != MAX_REG / 2; ++i)
    {
        if (watchAddr_[i] == 0)
        {
            n = i;
            return true;
        }
    }
    return false;
}



bool DebugRegsPPC::find_available_iabr(int& n) const
{
    for (unsigned i = MAX_REG / 2; i != MAX_REG; ++i)
    {
        if (watchAddr_[i] == 0)
        {
            n = i;
            return true;
        }
    }
    return false;
}
