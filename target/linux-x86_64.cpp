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
#include "zdk/log.h"
#include <assert.h>
#include <iostream>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include "target/cpu.h"
#include "dharma/syscall_wrap.h"
#include "target/flags-i386.h"
#include "target/frame_reg.h"
#include "target/linux-i386.h"
#include "target/linux_live.h"
#include "target/reg.h"


#define GET_REG(type, regname) \
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));\
    type result = CHKPTR(r)->regname;\
    Platform::after_read(thread, result);\
    return result;

using namespace std;


long LinuxTarget::syscall_num(const Thread& thread) const
{
    GET_REG(long, orig_rax);
}


word_t LinuxTarget::result(const Thread& thread) const
{
    GET_REG(word_t, rax);
}


void LinuxTarget::set_result(Thread& thread, word_t value)
{
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), RAX, value);
}


int64_t LinuxTarget::result64(const Thread& thread) const
{
    GRegs* r = interface_cast<GRegs*>(this->regs(thread));

    int64_t result = CHKPTR(r)->rax;

    if (thread.is_32_bit())
    {
        result = (r->rdx << 32) + r->rax;
    }
    return result;
}


void LinuxTarget::set_result64(Thread& thread, int64_t value)
{
    if (thread.is_32_bit())
    {
        sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), RDX, value >> 32);
        sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), RAX, (value & 0xffffffff));
    }
    else
    {
        sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), RAX, value);
    }
}


long double
LinuxTarget::result_double(const Thread& thread, size_t size) const
{
    user_fpxregs_struct fpxregs;

    memset(&fpxregs, 0, sizeof fpxregs);
    sys::get_fpxregs(thread.lwpid(), fpxregs);

    if (thread.is_32_bit())
    {
        long double val = *reinterpret_cast<long double*>(&fpxregs.st_space);
        return val;
    }

    switch (size)
    {
    case sizeof(float):
        return *reinterpret_cast<float*>(&fpxregs.xmm_space);

    case sizeof(double):
        return *reinterpret_cast<double*>(&fpxregs.xmm_space);

    case sizeof(long double):
        return *reinterpret_cast<long double*>(&fpxregs.st_space);
    }

    ostringstream err;

    err << "unexpected floating point type size: " << size;
    throw logic_error(err.str());

    return .0; // keep compiler happy
}


void LinuxTarget::set_result_double(Thread& thread, long double value, size_t size)
{
    if (thread.is_32_bit())
    {
        clog << "Warning: setting " << value << " (" << size << ") bytes in 32bit target\n";
    }
    const pid_t lwpid = thread.lwpid();

    user_fpxregs_struct fpxregs;
    memset(&fpxregs, 0, sizeof fpxregs);
    sys::get_fpxregs(lwpid, fpxregs);

    memcpy(&fpxregs.st_space, &value, sizeof value);
    sys::ptrace(__ptrace_request(PTRACE_SETFPXREGS),
                                 lwpid, 0,
                                 reinterpret_cast<word_t>(&fpxregs));
}


addr_t LinuxTarget::program_count(const Thread& thread) const
{
    GET_REG(addr_t, rip);
}


addr_t LinuxTarget::frame_pointer(const Thread& thread) const
{
    GET_REG(addr_t, rbp);
}


addr_t LinuxTarget::stack_pointer(const Thread& thread) const
{
    GET_REG(addr_t, rsp);
}


void LinuxTarget::set_stack_pointer(Thread& thread, addr_t addr)
{
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), RSP, addr);
}



/**
 * Enumerate the general purpose registers, return number of
 * registers. If a non-null pointer to a callback object is given,
 * then call its notify() method for each register.
 */
size_t
LinuxTarget::enum_user_regs(const Thread& thread,
                            EnumCallback<Register*>* cb) const
{
    GRegs& r = interface_cast<GRegs&>(*CHKPTR(this->regs(thread)));
    std::vector<RefPtr<Register> > regs;

    if (thread.is_32_bit())
    {
        REG32(rbx, "ebx");
        REG32(rcx, "ecx");
        REG32(rdx, "edx");
        REG32(rsi, "rsi");
        REG32(rdi, "edi");
        FRAME_REG32(rbp, "ebp", frame_pointer);
        REG32(rax, "eax");
        REG32(ds,  "xds");
        REG32(es,  "xes");
        REG32(fs,  "xfs");
        REG32(gs,  "xgs");
        FRAME_REG32(rip, "eip", program_count);
        REG32(cs,  "xcs");
        FRAME_REG32(rsp, "esp", stack_pointer);
        REG32(ss,   "xss");

        regs.push_back(new Flags386("eflags", thread, OFF(eflags), r.eflags));
    }
    else
    {
        REG(r15);
        REG(r14);
        REG(r13);
        REG(r12);
        FRAME_REG(rbp, frame_pointer);
        REG(rbx);
        REG(r11);
        REG(r10);
        REG(r9);
        REG(r8);
        REG(rax);
        REG(rcx);
        REG(rdx);
        REG(rsi);
        REG(rdi);

        FRAME_REG(rip, program_count);
        REG(cs);

        regs.push_back(new Flags386("eflags", thread, OFF(eflags), r.eflags));

        FRAME_REG(rsp, stack_pointer);

        REG(ss);
        REG(fs_base);
        REG(gs_base);
        REG(ds);
        REG(es);
        REG(fs);
        REG(gs);
    }
#undef OFF
#undef USER_REG
#undef REG32

    if (cb)
    {
        for (size_t i(0); i != regs.size(); ++i)
        {
            cb->notify(regs[i].get());
        }
    }
    return regs.size();
}


#define OFF(x) (size_t)&(((user_fpxregs_struct*)0)->x)
#define FPU_REG(n) \
    regs.push_back(make_fpxreg(RNAME(n), thread, OFF(n), fpxregs.n));

#define OFF32(x) (size_t)&(((user_fpxregs_32*)0)->x)
#define FPU_REG32(n) \
    regs.push_back(make_fpxreg(RNAME(n), thread, OFF32(n), fpx32.n));

/**
 * Enumerate the general purpose registers, and the FPU
 * and XMM registers, in the same manner as enum_user_regs
 */
size_t LinuxTarget::enum_fpu_regs
(
    const Thread& thread,
    EnumCallback<Register*>* callback
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
    if (thread.is_32_bit())
    {
        FPXRegs32& fpx32 = interface_cast<FPXRegs32&>(*obj);

        FPU_REG32(cwd)
        FPU_REG32(swd)
        FPU_REG32(twd)
        FPU_REG32(fop)
        FPU_REG32(fip)
        FPU_REG32(fcs)
        FPU_REG32(foo)
        FPU_REG32(fos)
        FPU_REG32(mxcsr)
    }
    else
    {
        FPU_REG(cwd)
        FPU_REG(swd)
        FPU_REG(ftw)
        FPU_REG(fop)
        FPU_REG(rip)
        FPU_REG(rdp)
        FPU_REG(mxcsr)
        FPU_REG(mxcr_mask)
    }
    // ST(0) thru ST(7)
    for (size_t i(0); i != 8; ++i)
    {
        char name[8] = { 0 };
        snprintf(name, sizeof name - 1, "st%ld", i);
        long double value = thread.is_32_bit()
            ? *reinterpret_cast<const long double*>(&fpxregs.st_space[i * 4])
            : *reinterpret_cast<const double*>(&fpxregs.st_space[i * 4]);

        regs.push_back(new Reg<long double, REG_FPUX>(
            name, thread, OFF(st_space[i * 4]), value));
    }
    // the AMD64 architecture provides sixteen 128-bit XMM registers.
    // Registers XMM0 through XMM7 are used for passing float and double
    // parameters.
    //
    // NOTE: The long double type is passed in memory. A long double
    // in AMD64 architecture is 16 bytes long compared to 12 bytes in the
    // x86 architecture. A double is 8-bytes long.

    for (size_t i(0); i != 8; ++i)
    {
        char name[10];
        snprintf(name, sizeof name - 1, "xmm%ld", i);
        long double value = thread.is_32_bit()
            ? *reinterpret_cast<const long double*>(&fpxregs.xmm_space[i * 4])
            : *reinterpret_cast<const double*>(&fpxregs.xmm_space[i * 4]);

        regs.push_back(new Reg<long double, REG_FPUX>(
            name, thread, OFF(xmm_space[i * 4]), value));
    }
    if (callback)
    {
        for (size_t i(0); i != regs.size(); ++i)
        {
            callback->notify(regs[i].get());
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
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), RIP, addr);
}


bool LinuxTarget::read_register( const Thread& thread,
                                 int  nreg,
                                 bool useFrame,
                                 reg_t& regOut
                               ) const
{
    bool result = false;

    if (useFrame && thread.stack_trace_depth())
    {
        StackTrace* trace = const_cast<Thread&>(thread).stack_trace();
        assert(trace);
        if (thread.is_32_bit())
        {
            result = get_frame_reg32(trace, nreg, regOut);

            dbgout(0) << "get_frame_reg32(" << nreg << ")=" << result << endl;
        }
        else switch (nreg)
        {
        case RSP / 8:
            regOut = CHKPTR(trace->selection())->stack_pointer();
            result = true;
            break;

        case RBP / 8:
            regOut = CHKPTR(trace->selection())->frame_pointer();
            result = true;
            break;

        case RIP / 8:
            regOut = CHKPTR(trace->selection())->program_count();
            result = true;
            break;

        default:
            break;
        }
        Platform::after_read(thread, regOut);
    }
    return result;
}


/**
 * Architecture-dependent algorithm for passing parameters
 * by CPU registers
 */
bool LinuxLiveTarget::pass_by_reg(Thread& thread, std::vector<RefPtr<Variant> >& args)
{
    // use the following registers for passing ints, pointers, etc.
    static const unsigned int int_regs[] = {
        RDI / 8,
        RSI / 8,
        RDX / 8,
        RCX / 8,
        R8  / 8,
        R9  / 8,
    };

    static const unsigned int max_int_regs =
        sizeof(int_regs)/sizeof(int_regs[0]);

    // use xmm0 thru 5 for passing doubles and floats, as per
    // http://gcc.fyxm.net/summit/2003/Porting gcc to the amd64.pdf
    static const unsigned int max_float_regs = 6;

    if (thread.is_32_bit())
    {
        return false;
    }
    Runnable& runnable = interface_cast<Runnable&>(thread);

    // indices of next available integer and float registers,
    // respectively
    size_t nextIntReg = 0;
    size_t nextFloatReg = 0;

    vector<RefPtr<Variant> >::iterator ii = args.begin();
    for (ii = args.begin(); ii != args.end(); )
    {
        const RefPtr<Variant>& v = *ii;

        switch (v->type_tag())
        {
        case Variant::VT_INT8:
        case Variant::VT_UINT8:
        case Variant::VT_INT16:
        case Variant::VT_UINT16:
        case Variant::VT_INT32:
        case Variant::VT_UINT32:
        case Variant::VT_INT64:
        case Variant::VT_UINT64:
            // arguments of types (signed and unsigned) bool,
            // char, short, int, long, long long and pointers
            // are in the INTEGER class
            if (nextIntReg < max_int_regs)
            {
                uint64_t n = (*ii)->bits();
                size_t nreg = int_regs[nextIntReg++];
                dbgout(0) << n << " by reg: " << nreg << endl;
                runnable.write_register(nreg, n);
                ii = args.erase(ii);
                continue;
            }
            break;

        case Variant::VT_POINTER:
            if (nextIntReg < max_int_regs)
            {
                uint64_t n = (*ii)->pointer();
                runnable.write_register(int_regs[nextIntReg++], n);
                ii = args.erase(ii);
                continue;
            }
            break;

        case Variant::VT_FLOAT:
        case Variant::VT_DOUBLE:
            // arguments of types float, double, and __m64 are in
            // class SSE and passed via XMM registers
            if (nextFloatReg < max_float_regs)
            {
                user_fpxregs_struct fpxregs;
                sys::get_regs(thread.lwpid(), fpxregs);

                double d = v->long_double();
                *((double*)&fpxregs.xmm_space[nextFloatReg * 4]) = d;
                dbgout(0) << d << " by reg: " << nextFloatReg << endl;
                FPXRegs tmp(fpxregs);
                Runnable& task = interface_cast<Runnable&>(thread);
                task.set_registers(NULL, &tmp);

                ++nextFloatReg;
                ii = args.erase(ii);
                continue;
            }
            break;

        case Variant::VT_LONG_DOUBLE:
            break;

        case Variant::VT_OBJECT:
            if (DebugSymbol* sym = v->debug_symbol())
            {
                if (CHKPTR(sym->type())->size() > 16)
                {
                    break;
                }
            }
            // todo: small objects
            // fallthru

        default:
            {
                ostringstream msg;
                msg << "Sorry, passing argument of type ";
                if (DebugSymbol* sym = v->debug_symbol())
                {
                    msg << sym->type()->name();
                }
                else
                {
                    msg << v->type_tag();
                }
                msg << " is not yet supported for x86-64";
                throw runtime_error(msg.str());
            }
        }
        ++ii;
    }

    // reg %AL is expected to hold the number of SSE registers
    runnable.write_register(RAX / 8, nextFloatReg);

    return (nextIntReg || nextFloatReg);
}


addr_t LinuxLiveTarget::stack_reserve(Thread& thread, addr_t sp) const
{
    if (!thread.is_32_bit()) // AMD64?
    {
        sp -= 128; // red zone, mandated by the Itanium ABI
    }
    return sp;
}


addr_t LinuxLiveTarget::stack_align(Thread& thread, addr_t sp) const
{
    if (!thread.is_32_bit())
    {
        sp &= ~(addr_t)0xf;
        assert((sp % 16) == 0);
    }
    return sp;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
