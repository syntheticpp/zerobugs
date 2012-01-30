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
#include <iostream>
#include <sstream>
#include <sys/procfs.h>
#include "core_thread.h"
#include "debugger_base.h"
#include "dharma/exec_arg.h"
#include "dharma/virtual_dso.h"
#include "elfz/public/core_file.h"
#include "symbolz/public/symbol_map.h"
#include "zdk/thread_util.h"
#include "linux_core.h"
#include "target_factory.h"

using namespace std;

namespace
{
    typedef ELF::CoreFile::const_prstatus_iterator const_prstatus_iterator;
    typedef ELF::CoreFile::const_fpxregs_iterator const_fpxregs_iterator;

    static RefPtr<Target> create(debugger_type& debugger)
    {
        return new LinuxCoreTarget(debugger);
    }
}

void init_core_target()
{
    TheTargetFactory::instance().register_target(
        TargetFactory::Linux,
        __WORDSIZE,
        false,
        string(),
        create);
}


////////////////////////////////////////////////////////////////
LinuxCoreTarget::LinuxCoreTarget(debugger_type& debugger)
    : LinuxTarget(debugger)
    , kind_(K_UNKNOWN)
{
}


////////////////////////////////////////////////////////////////
LinuxCoreTarget::~LinuxCoreTarget() throw()
{
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::read_memory
(
    pid_t,
    SegmentType,
    addr_t        addr,
    word_t*       buf,
    size_t        len,
    size_t*       wordsRead
) const
{
    assert(core_);

    const size_t nbytes = len * sizeof(word_t);

    size_t bytesRead = core_->readmem(addr, (char*)buf, nbytes);

    dbgout(0) << "LinuxCoreTarget::" << __func__ << "=" << bytesRead << endl;
    assert((bytesRead % sizeof (word_t)) == 0);

    if (wordsRead)
    {
        *wordsRead = bytesRead / sizeof (word_t);
    }

}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::write_memory
(
    pid_t,
    SegmentType,
    addr_t,
    const word_t*,
    size_t
)
{
    throw logic_error("cannot write memory in a corefile");
}



////////////////////////////////////////////////////////////////
bool LinuxCoreTarget::has_linker_events() const
{
    return true;
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::attach(pid_t)
{
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::detach(bool)
{
}


////////////////////////////////////////////////////////////////
bool LinuxCoreTarget::stop_all_threads(Thread*)
{
    return false;
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::stop_async()
{
}


////////////////////////////////////////////////////////////////
size_t LinuxCoreTarget::resume_all_threads()
{
    return 0;
}


////////////////////////////////////////////////////////////////
size_t LinuxCoreTarget::cleanup(Thread&)
{
    return size();
}


////////////////////////////////////////////////////////////////
const ELF::CoreFile& LinuxCoreTarget::core() const
{
    CHKPTR(core_);
    return *core_;
}


////////////////////////////////////////////////////////////////
Thread* LinuxCoreTarget::exec(const ExecArg& arg, const char* const*)
{
    const char* prog = NULL;
    if (arg.empty())
    {
        throw runtime_error("empty core argument list");
    }
    if (arg.size() > 1)
    {
        prog = arg[1];
    }
#if __GNUC__ == 2
    CorePtr tmp(new ELF::CoreFile(arg[0], prog));
    core_ = tmp;
#else
    core_.reset(new ELF::CoreFile(arg[0], prog));
#endif
    init_process(core_->pid(), &arg, ORIGIN_CORE);
    init_symbols();

    const_prstatus_iterator i = core_->prstatus_begin();
    const const_prstatus_iterator end = core_->prstatus_end();
    for (size_t n = 0; i != end; ++i, ++n)
    {
        RefPtr<Thread> thread(new CoreThreadImpl(*this, core_, n, i->second));

        add_thread(thread);
        debugger().on_attach(*thread);
    }
    return 0;
}


////////////////////////////////////////////////////////////////
RefPtr<ProcessImpl>
LinuxCoreTarget::new_process(pid_t pid,
                             const ExecArg* arg,
                             ProcessOrigin // orig
                            )
{
    assert(core_); // pre-condition

    const char* proc = (arg->size() > 1) ? (*arg)[1] : NULL;
    if (!proc || !*proc)
    {
        proc = core_->process_name().c_str();
    }
    return new ProcessImpl(*this, pid, ORIGIN_CORE, NULL, proc);
}


////////////////////////////////////////////////////////////////
RefPtr<SymbolMap> LinuxCoreTarget::read_symbols()
{
    assert(process());

    return read_symbols_from_core_dump(
        *process(), *core_, *debugger().symbol_table_events());
}


////////////////////////////////////////////////////////////////
addr_t LinuxCoreTarget::setup_caller_frame(Thread&, addr_t, long)
{
    throw logic_error("cannot setup caller frame in a corefile");
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::set_program_count(Thread&, addr_t)
{
    throw logic_error("cannot alter the program counter in a corefile");
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::set_stack_pointer(Thread&, addr_t)
{
    throw logic_error("cannot alter the stack pointer in a corefile");
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::set_result(Thread&, word_t)
{
    throw logic_error("cannot alter register in a corefile");
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::set_result64(Thread&, int64_t)
{
    throw logic_error("cannot alter register in a corefile");
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::set_result_double(Thread&, long double, size_t)
{
    throw logic_error("cannot alter register in a corefile");
}


////////////////////////////////////////////////////////////////
ZObject* LinuxCoreTarget::regs(const Thread& thread) const
{
    ZObject* result = NULL;

    const_prstatus_iterator i = core().prstatus_find(thread.lwpid());
    if (i != core().prstatus_end())
    {
        const user_regs_struct& regs =
            reinterpret_cast<const user_regs_struct&>(i->second.pr_reg);

        RefPtr<GRegs> r(new GRegs(regs));
        result = thread.regs(r.get());
    }

    assert(result);
    return result;
}


////////////////////////////////////////////////////////////////
ZObject* LinuxCoreTarget::fpu_regs(const Thread& thread) const
{
    ZObject* result = thread.fpu_regs();
    if (!result)
    {

        // assume that the fpxreg notes are in the same
        // order in the core file as prstatus entries

        const_fpxregs_iterator i(core().fpxregs_begin());
        const_prstatus_iterator j = core().prstatus_begin();
        for (; j != core().prstatus_end(); ++i, ++j)
        {
            if (i == core().fpxregs_end())
            {
                break;
            }
            if (j->second.pr_pid == thread.lwpid())
            {
                RefPtr<FPXRegs> r(new FPXRegs(*i));
                result = thread.fpu_regs(r.get());
                break;
            }
        }
    }
    return result;
}


/**
 * @return the value stored in the N-th general purpose register
 */
bool
LinuxCoreTarget::read_register( const Thread& thread,
                                int nreg,
                                bool readFrame,
                                reg_t& rout
                              ) const
{
    bool result = LinuxTarget::read_register(thread, nreg, readFrame, rout);

    if (!result && core_)
    {
        const_prstatus_iterator i = core_->prstatus_find(thread.lwpid());
        if (i != core_->prstatus_end())
        {
#ifdef HAVE_PRSTATUS_GREGSETSZ
            const size_t ELF_NGREG = i->second.pr_gregsetsz / sizeof(reg_t);
#endif
            if (nreg < 0 || static_cast<size_t>(nreg) >= ELF_NGREG)
            {
                ostringstream err;

                err << __func__ << ": " << nreg;
                err << " out of range, max is " << ELF_NGREG;

                throw out_of_range(err.str());
            }
            rout = GENERAL_REG(i->second.pr_reg, nreg);

            Platform::after_read(thread, rout);
            result = true;
        }
    }

    return result;
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::write_register(Thread&, size_t, reg_t)
{
    throw logic_error("cannot alter registers in a corefile");
}


////////////////////////////////////////////////////////////////
bool LinuxCoreTarget::write_register(Register&, const Variant&)
{
    throw logic_error("cannot alter registers in a corefile");
}


////////////////////////////////////////////////////////////////
VirtualDSO* LinuxCoreTarget::read_virtual_dso() const
{
    VirtualDSO* result = 0;

    if (process() && core_.get())
    {
        addr_t addr = 0;
        size_t size = 0;

        const AuxVect& v = core_->auxv();

        if (get_sysinfo_ehdr(v, addr, size))
        {
          /*
            if (kind() == ELFCLASS32)
            {
                addr &= 0xffffffff;
            }
          */
            dbgout(0) << __func__ << ": addr=" << (void*)addr << endl;

            std::vector<char> buf(size);

            const size_t nread = core_->readmem(addr, &buf[0], size);
            if (nread == size)
            {
                result = new VirtualDSO(buf, addr);
            }
            else
            {
                dbgout(0) << __func__ << ": readmem=" << nread << endl;
            }
        }
    }
    dbgout(0) << __func__ << "=" << result << endl;
    return result;
}


////////////////////////////////////////////////////////////////
void LinuxCoreTarget::read_environment(SArray& env) const
{
    CHKPTR(core_)->get_environment(env);
}


////////////////////////////////////////////////////////////////
const string& LinuxCoreTarget::command_line() const
{
    return CHKPTR(core_)->name();
}


////////////////////////////////////////////////////////////////
Target::Kind LinuxCoreTarget::kind() const
{
    if (kind_ == K_UNKNOWN)
    {
        if (core_)
        {
            switch (core_->header().klass())
            {
            case ELFCLASS32:
                kind_ = K_NATIVE_32BIT;
                break;

            case ELFCLASS64:
                kind_ = K_NATIVE_64BIT;
                break;
            }
        }
    }
    return kind_;
}

////////////////////////////////////////////////////////////////
word_t LinuxCoreTarget::get_breakpoint_opcode(word_t) const
{
    throw logic_error("cannot set breakpoints in corefiles");
}


////////////////////////////////////////////////////////////////
addr_t LinuxCoreTarget::stack_reserve(Thread&, addr_t sp) const
{
    throw logic_error("cannot reserve stack in corefiles");
    return sp;
}


////////////////////////////////////////////////////////////////
addr_t LinuxCoreTarget::stack_align(Thread&, addr_t sp) const
{
    throw logic_error("cannot align stack in corefiles");
    return sp;
}


////////////////////////////////////////////////////////////////
auto_ptr<DebugRegsBase> LinuxCoreTarget::get_debug_regs(Thread&) const
{
    //
    // Even if it were possible to read the values of the debug regs
    // from a core dump (albeit I seriously doubt that the kernel writes them
    // out) it would not be of any practical use.
    //
    throw logic_error("get_debug_regs: not implemented for core targets");
}


////////////////////////////////////////////////////////////////
string LinuxCoreTarget::thread_name(pid_t) const
{
    return core_->process_name(); // todo
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
