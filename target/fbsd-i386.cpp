// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <assert.h>
#include <sys/param.h>
#include <sys/user.h>
#include <machine/reg.h>
#include "cpu.h"
#include "debugger_base.h"
#include "fbsd.h"
#include "reg.h"
#include "zdk/check_ptr.h"


// general purpose registers
typedef Regs<user_regs_struct> GRegs;

// FP and extended registers
typedef FpuRegs<user_fpxregs_struct> FPXRegs;


////////////////////////////////////////////////////////////////
word_t FreeBSDTarget::result(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->r_eax;
}

////////////////////////////////////////////////////////////////
int64_t FreeBSDTarget::result64(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    union
    {
        struct { uint32_t low; uint32_t high; };
        int64_t value;
    } result;

    result.low = CHKPTR(r)->r_eax;
    result.high = CHKPTR(r)->r_edx;

    return result.value;
}

////////////////////////////////////////////////////////////////
long double
FreeBSDTarget::result_double(const Thread& thread, size_t size) const
{
    assert(false); // todo
    return .0;
}

////////////////////////////////////////////////////////////////
addr_t FreeBSDTarget::program_count(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->r_eip;
}

////////////////////////////////////////////////////////////////
addr_t FreeBSDTarget::frame_pointer(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->r_ebp;
}

////////////////////////////////////////////////////////////////
addr_t FreeBSDTarget::stack_pointer(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->r_esp;
}

////////////////////////////////////////////////////////////////
void FreeBSDTarget::set_result(Thread& thread, word_t)
{
    assert(false); // todo
}

////////////////////////////////////////////////////////////////
void FreeBSDTarget::set_result64(Thread& thread, int64_t value)
{
    assert(false); // todo
}

////////////////////////////////////////////////////////////////
void
FreeBSDTarget::set_result_double(Thread& thread,
                                 long double value,
                                 size_t size)
{
    assert(false); // todo
}

////////////////////////////////////////////////////////////////
void
FreeBSDTarget::set_program_count(Thread& thread, addr_t value)
{
    assert(false); // todo
}

////////////////////////////////////////////////////////////////
void
FreeBSDTarget::set_stack_pointer(Thread& thread, addr_t value)
{
    assert(false); // todo
}

////////////////////////////////////////////////////////////////
void FreeBSDTarget::set_registers(Thread&, ZObject*, ZObject*)
{
    assert(false); // todo
}

/**
 * map PID to Thread in which event has occurred
 */
Thread* FreeBSDTarget::event_pid_to_thread(pid_t) const
{
    assert(false);
    return 0; // todo
}

/**
 * thread ID to light-weight process ID
 */
pid_t FreeBSDTarget::tid_to_lwpid(long) const
{
    assert(false);
    return 0; // todo
}

////////////////////////////////////////////////////////////////
#define RNAME(x) #x

/**
 * Enumerate the general purpose registers, return number of
 * registers. If a non-null pointer to a callback object is given,
 * then call its notify() method for each register.
 */
size_t FreeBSDTarget::enum_user_regs(
    const Thread& thread,
    EnumCallback<Register*>* cb) const
{
#define OFF(x) (size_t)&(((user_regs_struct*)0)->x)
#define USER_REG(n) \
regs.push_back( \
    new Reg<reg_t, REG_USER>(RNAME(n), thread, OFF(n), r.n));

    GRegs& r = interface_cast<GRegs&>(*CHKPTR(this->regs(thread)));

    std::vector<RefPtr<Register> >regs;

    USER_REG(r_ebx)
    USER_REG(r_ecx)
    USER_REG(r_edx)
    USER_REG(r_esi)
    USER_REG(r_edi)
    USER_REG(r_ebp)
    USER_REG(r_eax)
    USER_REG(r_xds)
    USER_REG(r_xes)
    USER_REG(r_xfs)
    USER_REG(r_xgs)
    USER_REG(r_eip)
    USER_REG(r_xcs)
    USER_REG(r_esp)
    USER_REG(r_xss)
    USER_REG(r_eflags)

#undef RINDX
#undef USER_REG

    if (cb)
    {
        for (size_t i(0); i != regs.size(); ++i)
        {
            cb->notify(regs[i].get());
        }
    }
    return regs.size();
}

/**
 * Enumerate the general purpose registers, and the FPU
 * and XMM registers, in the same manner as enum_user_regs
 */
size_t FreeBSDTarget::enum_fpu_regs(
    const Thread& thread,
    EnumCallback<Register*>* callback) const
{
    const FPXRegs* fpr =
        interface_cast<FPXRegs*>(this->fpu_regs(thread));

    if (!fpr)
    {
        return 0;
    }
    if (callback)
    {
        // env87
        //const env87& env =
        //    reinterpret_cast<const env87&>(fpr->fpr_env);

        // todo todo todo
    }
    return 0;
}
#undef RNAME


Target::Kind FreeBSDTarget::kind() const
{
    return K_NATIVE_32BIT;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
