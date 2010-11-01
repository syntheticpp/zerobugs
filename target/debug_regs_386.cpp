//
// $Id: debug_regs_386.cpp 720 2010-10-28 06:37:54Z root $
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
#include "zdk/xtrace.h"
#include <signal.h>

#ifndef HAVE_DEBUG_REGS_386
 #error incorrect file for this platform
#endif
#include "debug_regs_386.h"

#include <bitset>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include "zdk/zero.h"
#include "dharma/syscall_wrap.h"
#include "generic/state_saver.h"


#define OFFSET_OF(S, f) reinterpret_cast<size_t>(& ((S*)0)->f )

#define LOCAL_EXACT         0x100
#define GLOBAL_EXACT        0x200


using namespace std;

static inline reg_t
len_type(DebugRegs::Length n,
         DebugRegs::Condition c,
         uint32_t regIndex)
{
    return ((n << 2) | c) << (regIndex * 4 + 16);
}

static inline reg_t enable_local(uint32_t regIndex)
{
    return (1 << (regIndex << 1));
}

static inline reg_t enable_global(uint32_t regIndex)
{
    return (2 << (regIndex << 1));
}

static inline reg_t enable_all(uint32_t regIndex)
{
    return (3 << (regIndex << 1));
}

static inline reg_t get_condition(reg_t dreg7, uint32_t regIndex)
{
    return (dreg7 >> (16 + regIndex * 4)) & 3;
}


////////////////////////////////////////////////////////////////
DebugRegs386::DebugRegs386(Thread& thread)
    : thread_(thread)
    , control_(0)
{
    memset(reg_, 0, sizeof(reg_));
}


////////////////////////////////////////////////////////////////
bool DebugRegs386::set_breakpoint (
    addr_t      addr,
    uint32_t*   index,
    bool        global,
    Condition   cond,
    Length      length
)
{
    bool result = false;

    // look for an available address register
    for (size_t i = 0; i != DEBUG_REG_NUM; ++i)
    {
        if (reg_[i] == 0)
        {
    #ifdef HAVE_PTRACE_DBREGS
            struct dbreg dr;
            sys::ptrace(PT_GETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
            enable_internal(i, true, global, cond, length);
            dr.dr[i] = addr;
            dr.dr[7] = control_;
            sys::ptrace(PT_SETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
    #else
            // write address register
            const size_t off = OFFSET_OF(user, u_debugreg[i]);
            sys::ptrace(PTRACE_POKEUSER, thread_.lwpid(), off, addr);
            enable_internal(i, true, global, cond, length);
    #endif // !HAVE_PTRACE_DBREGS

            reg_[i] = addr;

            if (index)
            {
                *index = i;
            }
            result = true;
            break;
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
void DebugRegs386::clear(reg_t i)
{
    enable_internal(i, false,
        // the last 3 args don't matter here
        true, BREAK_ON_DATA_READ_WRITE, BREAK_FOUR_BYTE);

    assert(i < DEBUG_REG_NUM);

    const reg_t sr = this->status();
    bitset<DEBUG_REG_NUM> flags(sr & ((1 << DEBUG_REG_NUM) - 1));
    flags.set(i, false);

#ifdef HAVE_PTRACE_DBREGS
    dbreg dr;
    sys::ptrace(PT_GETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
    dr.dr[6] = flags.to_ulong();
    dr.dr[7] = control_;
    dr.dr[i] = 0;
    sys::ptrace(PT_SETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
#else
    size_t off = OFFSET_OF(user, u_debugreg[i]);
    sys::ptrace(PTRACE_POKEUSER, thread_.lwpid(), off, 0);

    off = OFFSET_OF(user, u_debugreg[6]);
    sys::ptrace(PTRACE_POKEUSER, thread_.lwpid(), off, flags.to_ulong());

#endif // !HAVE_PTRACE_DBREGS

    reg_[i] = 0;
}



////////////////////////////////////////////////////////////////
void DebugRegs386::enable_internal(
    uint32_t    reg,
    bool        activate,
    bool        global,
    Condition   cond,
    Length      len)
{
    if (reg >= DEBUG_REG_NUM)
    {
        throw std::out_of_range("DebugRegs: index is out of range");
    }

    if (activate)
    {
        reg_t mask = (3 << (reg << 1)) |
            len_type(BREAK_FOUR_BYTE, BREAK_ON_DATA_READ_WRITE, reg);
        control_ &= ~mask;

        mask = global ? enable_global(reg) : enable_local(reg);

        control_ |= len_type(len, cond, reg) | mask;

        /* make sure the "exact address" feature is enabled
           NOTE: this slows down the instruction pipeline */
        control_ |= (LOCAL_EXACT | GLOBAL_EXACT);
    }
    else
    {
        /* disable local and global alike */
        const reg_t mask = enable_global(reg) | enable_local(reg);
        control_ &= ~mask;

        /* if no more hardware breakpoints, remove the
           slowdown flags */
        if ((control_ & 0xFF) == 0)
        {
            control_ &= ~(GLOBAL_EXACT | LOCAL_EXACT);
        }
    }

#ifndef HAVE_PTRACE_DBREGS
    const size_t off = OFFSET_OF(user, u_debugreg[7]);
    sys::ptrace(PTRACE_POKEUSER, thread_.lwpid(), off, control_);
#endif
}


////////////////////////////////////////////////////////////////
void DebugRegs386::enable (
    uint32_t    reg,
    bool        activate,
    bool        global,
    Condition   cond,
    Length      len)
{
    enable_internal(reg, activate, global, cond, len);

#ifdef HAVE_PTRACE_DBREGS
    dbreg dr;
    sys::ptrace(PT_GETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
    dr.dr[7] = control_;
    sys::ptrace(PT_SETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
#endif // HAVE_PTRACE_DBREGS
}


////////////////////////////////////////////////////////////////
addr_t DebugRegs386::addr_in_reg(reg_t i) const
{
    if (i >= DEBUG_REG_NUM)
    {
        throw out_of_range("DebugRegs: index is out of range");
    }
    return reg_[i];
}


////////////////////////////////////////////////////////////////
addr_t DebugRegs386::hit(reg_t* condition)
{
    addr_t result = 0;
    const reg_t statReg = this->status();

#if 0 // experiment
    siginfo_t siginfo = { 0 };

    if (XTrace::ptrace(PT_GETSIGINFO, thread_.lwpid(), 0, addr_t(&siginfo)) == 0)
    {
        if (siginfo.si_signo == SIGTRAP)
        {
            clog << "***** " << siginfo.si_addr << " *****\n";
        }
    }
#endif
    bitset<DEBUG_REG_NUM> flags(statReg & ((1 << DEBUG_REG_NUM) - 1));
    if (flags.any())
    {
        for (unsigned int i = 0; i != DEBUG_REG_NUM; ++i)
        {
            if (flags.test(i))
            {
                flags.set(i, false);
                result = addr_in_reg(i);

                if (result == 0)
                {
                #if DEBUG && !HAVE_PTRACE_DBREGS

                    addr_t DR7 = OFFSET_OF(user, u_debugreg[7]);
                    DR7 = sys::ptrace(PTRACE_PEEKUSER, thread_.lwpid(), DR7, 0);

                    clog << "##### "<< thread_.lwpid() << ": DR7=" << hex;
                    clog << DR7 << " status=" << statReg << dec << endl;
                #endif
                }
                else
                {
                    if (condition)
                    {
                        *condition = get_condition(control_, i);
                    }
                }
                break;
            }
        }
        if (result)
        {
            // reset DR6
    #ifdef HAVE_PTRACE_DBREGS
            struct dbreg dr;
            memset(&dr, 0, sizeof dr);
            sys::ptrace(PT_GETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
            dr.dr[6] = flags.to_ulong();
            sys::ptrace(PT_SETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
    #else
            const size_t off = OFFSET_OF(user, u_debugreg[6]);
            const reg_t newval = flags.to_ulong();
            sys::ptrace(PTRACE_POKEUSER, thread_.lwpid(), off, newval);
    #endif
        }
    }
    return result;
}


///////////////////////////////////////////////////////////////
reg_t DebugRegs386::status() const
{
#ifdef HAVE_PTRACE_DBREGS
    struct dbreg dr;
    sys::ptrace(PT_GETDBREGS, thread_.lwpid(), (addr_t)&dr, 0);
    return dr.dr[6];
#else
    const size_t off = OFFSET_OF(user, u_debugreg[6]);
    reg_t rstat = sys::ptrace(PTRACE_PEEKUSER, thread_.lwpid(), off, 0);

    return rstat;
#endif
}


////////////////////////////////////////////////////////////////
reg_t DebugRegs386::control() const
{
    return control_;
}


////////////////////////////////////////////////////////////////
void DebugRegs386::dump(ostream& outs) const
{
#ifdef HAVE_PTRACE_DBREGS
    //
    // todo
    //
#else
    StateSaver<ios, ios::fmtflags> flags(outs);
    const size_t w = sizeof(reg_t) * 2;
    const pid_t lwpid = thread_.lwpid();
    outs << "--- Debug Registers [" << lwpid << "] ---\n";
    size_t off = OFFSET_OF(user, u_debugreg[7]);
    reg_t reg = sys::ptrace(PTRACE_PEEKUSER, lwpid, off, 0);
    bitset<sizeof(reg) * 8> bits(reg);
    outs << "DR7: " << hex << setw(w) << reg << " (" << bits << ")\n";

    reg = status();

    bits = reg;
    outs << "DR6: " << hex << setw(w) << reg << " (" << bits << ")\n";
    for (size_t i = 4; i > 0; --i)
    {
        size_t j = i - 1;
        off = OFFSET_OF(user, u_debugreg[j]);
        reg = sys::ptrace(PTRACE_PEEKUSER, lwpid, off, 0);
        outs << "DR" << j << ": " << hex << setw(w) << reg << endl;
    }
#endif
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
