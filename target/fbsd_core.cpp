//
// $Id: fbsd_core.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/stdexcept.h"
#include "dharma/exec_arg.h"
#include "fbsd_core.h"
#include "target_factory.h"


using namespace std;


static RefPtr<Target> create(debugger_type& debugger)
{
    return new FreeBSDCoreTarget(debugger);
}

void init_core_target()
{
    TheTargetFactory::instance().register_target(
        TargetFactory::FreeBSD,
        __WORDSIZE,
        false,
        string(),
        create);
}


FreeBSDCoreTarget::FreeBSDCoreTarget(debugger_type& dbg)
    : FreeBSDTarget(dbg)
{
}


FreeBSDCoreTarget::~FreeBSDCoreTarget() throw()
{
}


Thread* FreeBSDCoreTarget::exec(const ExecArg&, const char* const*)
{
    throw std::logic_error("exec not supported for core files");
}


void FreeBSDCoreTarget::detach(bool no_throw)
{
    throw std::logic_error("not implemented");
}


RefPtr<SymbolMap> FreeBSDCoreTarget::read_symbols()
{
    throw std::logic_error("not implemented");
    return NULL;
}


void FreeBSDCoreTarget::set_result(Thread&, word_t)
{
    assert(false); // todo
}


void FreeBSDCoreTarget::set_result64(Thread&, int64_t)
{
    assert(false); // todo
}


void
FreeBSDCoreTarget::set_result_double(Thread&, long double, size_t)
{
    assert(false); // todo
}


void FreeBSDCoreTarget::set_program_count(Thread&, addr_t)
{
    assert(false); // todo
}


void FreeBSDCoreTarget::set_stack_pointer(Thread&, addr_t)
{
    assert(false); // todo
}


void FreeBSDCoreTarget::set_registers(Thread&, ZObject*, ZObject*)
{
    assert(false); // todo
}


void FreeBSDCoreTarget::attach(pid_t)
{
    assert(false); // todo
}


bool FreeBSDCoreTarget::stop_all_threads(Thread*)
{
    assert(false);
    return false;
}


size_t FreeBSDCoreTarget::resume_all_threads()
{
    assert(false);
    return 0;
}


void FreeBSDCoreTarget::stop_async()
{
    assert(false);
}


bool FreeBSDCoreTarget::has_linker_events() const
{
    assert(false);
    return false;
}


size_t FreeBSDCoreTarget::cleanup(Thread&)
{
    assert(false);
    return 0;
}

void FreeBSDCoreTarget::write_register(Thread&, size_t, reg_t)
{
    throw logic_error("cannot alter registers in a corefile");
}

bool FreeBSDCoreTarget::write_register(Register&, const Variant&)
{
    throw logic_error("cannot alter registers in a corefile");
}

bool
FreeBSDCoreTarget::read_register(const Thread&,
                                 int regnum,
                                 bool fromStackFrame,
                                 reg_t& regOut) const
{
    assert(false); // todo
    return false;
}

ZObject* FreeBSDCoreTarget::regs(const Thread&) const
{
    assert(false);
    return 0;
}


ZObject* FreeBSDCoreTarget::fpu_regs(const Thread&) const
{
    assert(false);
    return 0;
}

void FreeBSDCoreTarget::read_memory(
    pid_t       pid,
    SegmentType segType,
    addr_t      addr,
    long*       buf,
    size_t      buflen, // length of buffer in machine words
    size_t*     wordsRead) const
{
    if (wordsRead)
    {
        *wordsRead = 0;
    }

    assert(false);// todo
}


void FreeBSDCoreTarget::write_memory(
    pid_t       pid,
    SegmentType segType,
    addr_t      addr,
    const long* buf,
    size_t      nwords
)
{
    throw logic_error("cannot alter memory in a corefile");
}


word_t FreeBSDCoreTarget::get_breakpoint_opcode(word_t) const
{
    throw logic_error("cannot set breakpoints in a corefile");
}

void FreeBSDCoreTarget::read_environment(SArray&) const
{
    // todo
}

RefPtr<ProcessImpl>
FreeBSDCoreTarget::new_process(pid_t pid, const ExecArg* arg, ProcessOrigin)
{
    const char* proc = (arg->size() > 1) ? (*arg)[1] : NULL;
/*
    if (!proc || !*proc)
    {
        proc = core_->process_name().c_str();
    }
 */
    return new ProcessImpl(*this, pid, ORIGIN_CORE, NULL, proc);
}

unsigned long FreeBSDCoreTarget::lwpid_to_tid(pid_t) const
{
    return 0; // todo
}

bool FreeBSDCoreTarget::read_state(const Thread&, RunnableState&) const
{
    return false;
}

pid_t FreeBSDCoreTarget::get_signal_sender_pid(const Thread&) const
{
    return -1; // unknown
}

addr_t FreeBSDCoreTarget::setup_caller_frame(Thread&, addr_t, long)
{
    throw logic_error("cannot setup caller frame in a corefile");
}


auto_ptr<DebugRegsBase> FreeBSDCoreTarget::get_debug_regs(Thread&) const
{
    //
    // Even if it were possible to read the values of the debug regs
    // from a core dump (albeit I seriously doubt that the kernel writes them
    // out) it would not be of any practical use.
    //
    throw logic_error("get_debug_regs: not implemented for core targets");
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
