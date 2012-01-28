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

#include <errno.h>
#include "zdk/config.h"
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "debug_out.h"
#include "dharma/syscall_wrap.h"
#include "engine/thread.h"
#include "target/cpu.h"
#include "interp.h"
#include "debug_out.h"
#include "cpu_state_saver.h"

using namespace std;


namespace
{
    class ThreadStateSaver : public EnumCallback<Thread*>
    {
        void notify(Thread*);
    };
    class ThreadStateRestorer : public EnumCallback<Thread*>
    {
        void notify(Thread*);
    };
}

typedef Regs<user_regs_struct> REGS;
typedef FpuRegs<user_fpxregs_struct> FPX_REGS;


////////////////////////////////////////////////////////////////
CPUStateSaver::CPUStateSaver(Thread& currentThread)
    : thread_(&currentThread)
    , restored_(false)
    , signal_(currentThread.signal())
{
    ThreadStateSaver stateSaver;
    CHKPTR(currentThread.debugger())->enum_threads(&stateSaver);

    // cancel pending syscall, if any
    if (currentThread.is_syscall_pending(currentThread.program_count()))
    {
        currentThread.cancel_syscall();
    }

    // according to http://www.agner.org/assem/calling_conventions.pdf
    // callee needs to save rbx, rbp, r12-r15
}


////////////////////////////////////////////////////////////////
CPUStateSaver::~CPUStateSaver() throw()
{
    try
    {
        restore_state();
    }
    catch (...)
    {
    }
}


////////////////////////////////////////////////////////////////
void CPUStateSaver::restore_state()
{
    if (!restored_)
    {
        ThreadStateRestorer stateRestorer;
        CHKPTR(thread_->debugger())->enum_threads(&stateRestorer);

        thread_->set_signal(signal_);
        DEBUG_OUT << "restored signal " << signal_ << endl;

        restored_ = true;
    }
}


////////////////////////////////////////////////////////////////
void ThreadStateSaver::notify(Thread* thread)
{
    // pre-conditions
    assert(thread);
    assert(thread->is_live());

    const pid_t tid = thread->lwpid(); // kernel thread id
    try
    {
        word_t depth = 0;
        thread->get_user_data(".regs", &depth);
        thread->set_user_data(".regs", ++depth);

        if (depth > 1)
        {
            return;
        }
        // save general regs
        RefPtr<REGS> regs = new REGS;
        sys::get_regs(tid, *regs);
        thread->set_user_object(".regs", regs.get());

        // save fpx_regs
        RefPtr<FPX_REGS> fpxRegs = new FPX_REGS;
        sys::get_fpxregs(tid, *fpxRegs);
        thread->set_user_object(".fpxregs", fpxRegs.get());

        DEBUG_OUT << thread->lwpid() << ": registers saved, SP="
                  << (void*)thread->stack_pointer() << endl;
    }
    catch (const exception& e)
    {
        cerr << "*** Warning: error saving state for thread ";
        cerr << thread->lwpid() << endl << e.what() << endl;
    }
}


////////////////////////////////////////////////////////////////
void ThreadStateRestorer::notify(Thread* thread)
{
    assert(thread);
    const pid_t pid = thread->lwpid();

    try
    {
        word_t depth = 0;

        if (thread->get_user_data(".regs", &depth))
        {
            assert(depth);
        }
        if (!depth)
        {
            return;
        }
        thread->set_user_data(".regs", depth - 1);

        if (depth > 1)
        {
            return;
        }

        interface_cast<Runnable&>(*thread).set_registers(
            thread->get_user_object(".regs"),
            thread->get_user_object(".fpxregs"));

        thread->set_user_object(".regs", 0);
        thread->set_user_object(".fpxregs", 0);

        DEBUG_OUT << pid << ": registers restored, SP="
                  << hex << thread->stack_pointer() << " PC="
                  << hex << thread->program_count() << dec
                  << endl;
    }
    catch (const exception& e)
    {
        cerr << "*** Warning: could not restore thread state: ";
        cerr << pid << endl << e.what() << endl;
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
