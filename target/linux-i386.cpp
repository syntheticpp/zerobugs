//
// $Id: linux-i386.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <assert.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include "dharma/syscall_wrap.h"
#include "target/flags-i386.h"
#include "target/frame_reg.h"
#include "target/linux-i386.h"
#include "target/linux_live.h"
#include "target/reg.h"


using namespace std;


long LinuxTarget::syscall_num(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->orig_eax;
}


word_t LinuxTarget::result(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->eax;
}


void LinuxTarget::set_result(Thread& thread, word_t val)
{
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), EAX * sizeof(reg_t), val);
}


int64_t LinuxTarget::result64(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    union
    {
        struct { uint32_t low; uint32_t high; };
        int64_t value;
    } result;

    result.low = CHKPTR(r)->eax;
    result.high = CHKPTR(r)->edx;

    return result.value;
}


void LinuxTarget::set_result64(Thread& thread, int64_t value)
{
    const pid_t pid = thread.lwpid();

    union
    {
        struct { uint32_t low; uint32_t high; };
        int64_t value;
    } result;

    result.value = value;

    sys::ptrace(PTRACE_POKEUSER, pid, EAX * sizeof(reg_t), result.low);
    sys::ptrace(PTRACE_POKEUSER, pid, EDX * sizeof(reg_t), result.high);
}


long double
LinuxTarget::result_double(const Thread& thread, size_t) const
{
    user_fpxregs_struct fpxregs;
    memset(&fpxregs, 0, sizeof fpxregs);
    sys::get_fpxregs(thread.lwpid(), fpxregs);

    return *(long double*)&fpxregs.st_space;
}


void
LinuxTarget::set_result_double(Thread& thread, long double value, size_t)
{
    const pid_t pid = thread.lwpid();

    user_fpxregs_struct fpxregs;
    memset(&fpxregs, 0, sizeof fpxregs);
    sys::get_fpxregs(pid, fpxregs);

    memcpy(&fpxregs.st_space, &value, sizeof value);
    sys::ptrace(__ptrace_request(PTRACE_SETFPXREGS),
                pid, 0, reinterpret_cast<word_t>(&fpxregs));
}


addr_t LinuxTarget::program_count(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->eip;
}


addr_t LinuxTarget::frame_pointer(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->ebp;
}


addr_t LinuxTarget::stack_pointer(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->esp;
}


void LinuxTarget::set_stack_pointer(Thread& thread, addr_t addr)
{
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), UESP * sizeof(reg_t), addr);
}


/**
 * Enumerate the general purpose registers, return number of
 * registers. If a non-null pointer to a observer object is given,
 * then call its notify() method for each register.
 */
size_t
LinuxTarget::enum_user_regs( const Thread& thread,
                             EnumCallback<Register*>* observer
                           ) const
{
    ZObject* obj = this->regs(thread);
    if (!obj)
        return 0;
    GRegs& r = interface_cast<GRegs&>(*obj);
    std::vector<RefPtr<Register> >regs;

    REG(ebx);   REG(ecx);   REG(edx);
    REG(esi);   REG(edi);   FRAME_REG(ebp,frame_pointer);
    REG(eax);   REG(xds);   REG(xes);
    REG(xfs);   REG(xgs);   FRAME_REG(eip,program_count);
    REG(xcs);   FRAME_REG(esp,stack_pointer);
    REG(xss);

    regs.push_back(new Flags386("eflags", thread, OFF(eflags), r.eflags));

    if (observer)
    {
        for (size_t i(0); i != regs.size(); ++i)
        {
            observer->notify(regs[i].get());
        }
    }
    return regs.size();
}
#undef OFF
#undef USER_REG
#undef FRAME_REG

#define OFF(x) (size_t)&(((user_fpxregs_struct*)0)->x)
#define FPU_REG(n) \
    regs.push_back(make_fpxreg(RNAME(n), thread, OFF(n), fpxregs.n));

/**
 * Enumerate the general purpose registers, and the FPU
 * and XMM registers, in the same manner as enum_user_regs
 */
size_t
LinuxTarget::enum_fpu_regs( const Thread& thread,
                            EnumCallback<Register*>* observer
                          ) const
{
    ZObject* obj = this->fpu_regs(thread);
    if (!obj)
        return 0;
    FPXRegs& fpxregs = interface_cast<FPXRegs&>(*obj);


    if (!this->fpu_regs(thread))
    {
        return 0;
    }
    std::vector<RefPtr<Register> >regs;

    FPU_REG(cwd)
    FPU_REG(swd)
    FPU_REG(twd)
    FPU_REG(fop)
    FPU_REG(fip)
    FPU_REG(fcs)
    FPU_REG(foo)
    FPU_REG(fos)
    FPU_REG(mxcsr)

    // ST(0) thru ST(7)
    for (size_t i = 0; i != 8; ++i)
    {
        char name[8] = { 0 };
        snprintf(name, sizeof name - 1, "st%d", i);
        long double value =
            *reinterpret_cast<const long double*>(
                &fpxregs.st_space[i * 4]);

        regs.push_back(new Reg<long double, REG_FPUX>(
            name, thread, OFF(st_space[i * 4]), value));
    }
    // XMM 0 thru 8
    for (size_t i(0); i != 8; ++i)
    {
        char name[10];
        snprintf(name, sizeof name - 1, "xmm%d", i);
        long double value =
            *reinterpret_cast<const long double*>(
                &fpxregs.xmm_space[i * 4]);
        regs.push_back(new Reg<long double, REG_FPUX>(
            name, thread, OFF(xmm_space[i * 4]), value));
    }

    if (observer)
    {
        for (size_t i(0); i != regs.size(); ++i)
        {
            observer->notify(regs[i].get());
        }
    }
    return regs.size();
}

#undef OFF
#undef FPU_REG
#undef RNAME


/**
 * Force execution to resume at given address,
 * by explicitly modifying the program counter
 */
void LinuxTarget::set_program_count(Thread& thread, addr_t addr)
{
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), EIP * sizeof(reg_t), addr);
}


bool
LinuxTarget::read_register( const Thread& thread,
                            int nreg,
                            bool useFrame,
                            reg_t& regOut
                          ) const
{
    if (useFrame && thread.stack_trace_depth())
    {
        if (StackTrace* trace = thread.stack_trace())
        {
            return get_frame_reg32(trace, nreg, regOut);
        }
    }
    //
    // the derived classes are expected to do the rest of the work
    //
    return false;
}



bool
LinuxLiveTarget::pass_by_reg(Thread&, std::vector<RefPtr<Variant> >&)
{
    return false;
}


addr_t LinuxLiveTarget::stack_reserve(Thread&, addr_t sp) const
{
    return sp;
}


addr_t LinuxLiveTarget::stack_align(Thread&, addr_t sp) const
{
    return sp;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
