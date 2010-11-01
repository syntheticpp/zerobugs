//
// $Id: linux-ppc.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "dharma/syscall_wrap.h"
#include "target/linux.h"
#include "target/linux_live.h"
#include "target/debug_regs_ppc.h"
#include "target/frame_reg.h"
#include "target/reg.h"
#include "zdk/check_ptr.h"

using namespace std;


long LinuxTarget::syscall_num(const Thread& thread) const
{
    const GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->orig_gpr3;
}


word_t LinuxTarget::result(const Thread& thread) const
{
    const GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->gpr[3];
}


void LinuxTarget::set_result(Thread& thread, word_t val)
{
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), PT_R3 * sizeof(reg_t), val);
}


int64_t LinuxTarget::result64(const Thread& thread) const
{
    const GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    union
    {
        struct { uint32_t low; uint32_t high; };
        int64_t value;
    } result;

    result.low = CHKPTR(r)->gpr[3];
    result.high = CHKPTR(r)->gpr[4];

    return result.value;
}


void LinuxTarget::set_result64(Thread& thread, int64_t value)
{
    throw logic_error(string(__func__) + " not implemented");
}


long double
LinuxTarget::result_double(const Thread& thread, size_t) const
{
    throw logic_error(string(__func__) + " not implemented");
}


void LinuxTarget::set_result_double(Thread&, long double value, size_t)
{
    throw logic_error(string(__func__) + " not implemented");
}


addr_t LinuxTarget::program_count(const Thread& thread) const
{
    const GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->nip; // next instruction pointer
}


addr_t LinuxTarget::frame_pointer(const Thread& thread) const
{
    const GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->gpr[31];
}


addr_t LinuxTarget::stack_pointer(const Thread& thread) const
{
    const GRegs* r = interface_cast<GRegs*>(this->regs(thread));
    return CHKPTR(r)->gpr[1];
}


void LinuxTarget::set_stack_pointer(Thread& thread, addr_t addr)
{
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), PT_R1 * sizeof(reg_t), addr);
}



static inline RefPtr<Register>
get_saved_reg_from_frame(const Frame& frame, const char* name)
{
    RefPtr<ZObject> obj = frame.get_user_object(name);
    return interface_cast<Register>(obj);
}


static RefPtr<Register>
get_register(const Thread& thread,
             const Frame* frame,
             const char* name,
             size_t n,
             const GRegs& r)
{
    RefPtr<Register> reg;

    if (frame)
    {
        reg = get_saved_reg_from_frame(*frame, name);
    }
    if (!reg)
    {
        reg = user_reg<reg_t>(thread, name, OFF(gpr[n]), r.gpr[n]);
    }
    return reg;
}


static RefPtr<Register>
get_link_register(const Thread& thread, const Frame* frame, const GRegs& r)
{
    if (frame)
    {
        size_t i = frame->index();
        try
        {
            frame = CHKPTR(thread.stack_trace())->frame(i + 1);

            const reg_t pc = CHKPTR(frame)->program_count();
            return user_reg<reg_t>(thread, "link", OFF(gpr[PT_LNK]), pc);
        }
        catch (...)
        { }
    }
    return REG_(link);
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
    // helper macros to populate the register vector
#define R(n) regs.push_back(get_register(thread, curFrame, "r" RNAME(n), n, r))
#define FR(n, f) regs.push_back(\
    user_reg<reg_t>(thread, "r" RNAME(n), OFF(gpr[n]), r.gpr[n], (&Frame::f)))
#undef REG
#define REG(n) regs.push_back(\
    get_register(thread, curFrame, RNAME(n), OFF(n)/sizeof(reg_t), r))

    ZObject* obj = this->regs(thread);
    if (!obj)
    {
        return 0;
    }
    const Frame* curFrame = thread_current_frame(&thread);

    GRegs& r = interface_cast<GRegs&>(*obj);
    std::vector<RefPtr<Register> >regs;

    // populate vector:
    R(0);
    FR(1, stack_pointer);
    R(2);
    R(3);
    R(4);
    R(5);
    R(6);
    R(7);
    R(8);
    R(9);
    R(10);
    R(11);
    R(12);
    R(13);
    R(14);
    R(15);
    R(16);
    R(17);
    R(18);
    R(19);
    R(20);
    R(21);
    R(22);
    R(23);
    R(24);
    R(25);
    R(26);
    R(27);
    R(28);
    R(29);
    R(30);
    FR(31, frame_pointer);

    FRAME_REG(nip, program_count);

    REG(msr);
    REG(ctr);

    //REG(link);
    regs.push_back(get_link_register(thread, curFrame, r));

    REG(xer);
    REG(ccr);
    REG(mq);
    REG(trap);
    REG(dar);
    REG(dsisr);
    REG(result); // syscall result

    if (observer)
    {
        for (size_t i(0); i != regs.size(); ++i)
        {
            observer->notify(regs[i].get());
        }
    }
    return regs.size();

#undef R
#undef FR
}

/**
 * Enumerate the general purpose registers, and the FPU
 * and XMM registers, in the same manner as enum_user_regs
 */
size_t
LinuxTarget::enum_fpu_regs( const Thread& thread,
                            EnumCallback<Register*>* observer
                          ) const
{
    return 0; // todo
}



/**
 * Force execution to resume at given address,
 * by explicitly modifying the program counter
 */
void LinuxTarget::set_program_count(Thread& thread, addr_t addr)
{
    sys::ptrace(PTRACE_POKEUSER, thread.lwpid(), PT_NIP * sizeof(reg_t), addr);
}


static inline bool
get_frame_reg(StackTrace& trace, size_t regIndex, reg_t& r)
{
    switch (regIndex)
    {
    case PT_R1:
        r = CHKPTR(trace.selection())->stack_pointer();
        return true;

    case PT_R31:
        r = CHKPTR(trace.selection())->frame_pointer();
        return true;

    case PT_NIP:
        r = CHKPTR(trace.selection())->program_count();
        return true;
    }
    return false;
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
        if (StackTrace* trace = const_cast<Thread&>(thread).stack_trace())
        {
            return get_frame_reg(*trace, nreg, regOut);
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


word_t LinuxLiveTarget::get_breakpoint_opcode(word_t word) const
{
    return 0x7d821008;
}


ZObject* LinuxLiveTarget::regs(const Thread& thread) const
{
    ZObject* result = thread.regs();
    if (!result)
    {
        RefPtr<GRegs> r(new GRegs);
        sys::get_regs(thread.lwpid(), *r);

        const pid_t lwpid = thread.lwpid();
        r->nip = sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_NIP * sizeof(reg_t));
        r->msr = sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_MSR * sizeof(reg_t));
        r->ctr = sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_CTR * sizeof(reg_t));
        r->link = sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_LNK * sizeof(reg_t));
        r->xer = sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_XER * sizeof(reg_t));
        r->ccr = sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_CCR * sizeof(reg_t));
        r->mq = sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_MQ * sizeof(reg_t));

    #ifdef __PPC64__
        r->trap =
            sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_TRAP * sizeof(reg_t));
        r->dar =
            sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_DAR * sizeof(reg_t));
        r->dsisr =
            sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_DSISR * sizeof(reg_t));
        r->result =
            sys::ptrace(PTRACE_PEEKUSER, lwpid, PT_RESULT * sizeof (reg_t));
    #endif

        result = thread.regs(r.get());
    }
    return result;
}


ZObject* LinuxLiveTarget::fpu_regs(const Thread& thread) const
{
    ZObject* result = thread.fpu_regs();
    if (!result)
    {
        RefPtr<FPXRegs> r(new FPXRegs);
        sys::get_fpxregs(thread.lwpid(), *r);

        result = thread.fpu_regs(r.get());
    }
    return result;
}


void LinuxLiveTarget::step_until_safe(Thread&, addr_t) const
{
}


addr_t
LinuxLiveTarget::setup_caller_frame(Thread& thread, addr_t sp, long pc)
{
    thread.write_data(sp, &pc, 1);
    sp -= sizeof (long);

    long lr = 0;
    thread.write_data(sp, &lr, 1);
    sp -= sizeof (long);

    return sp;
}


auto_ptr<DebugRegsBase> LinuxLiveTarget::get_debug_regs(Thread& thread) const
{
    return auto_ptr<DebugRegsBase>(new DebugRegsPPC(thread));
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
