#ifndef LINUX_CORE_H__05A17E5D_58A4_11DA_B82B_00C04F09BBCC
#define LINUX_CORE_H__05A17E5D_58A4_11DA_B82B_00C04F09BBCC
//
// $Id: linux_core.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/check_ptr.h"
#include "target/corefwd.h"
#include "target/linux.h"


class LinuxCoreTarget : public LinuxTarget
{
public:
    explicit LinuxCoreTarget(debugger_type&);

    ~LinuxCoreTarget() throw();

    DECLARE_VISITABLE()

    Kind kind() const;

    virtual word_t get_breakpoint_opcode(word_t) const;
    virtual addr_t stack_reserve(Thread&, addr_t) const;
    virtual addr_t stack_align(Thread&, addr_t) const;

    virtual void read_memory(
        pid_t,
        SegmentType,
        addr_t,
        word_t*,
        size_t,
        size_t*) const;

    virtual void write_memory(
        pid_t,
        SegmentType,
        addr_t,
        const word_t*,
        size_t);

    // Register access
    virtual bool read_register(const Thread&, int, bool, reg_t&) const;

    virtual void write_register(Thread&, size_t n, reg_t);

    virtual bool has_linker_events() const;

    virtual void attach(pid_t);

    virtual void detach(bool);

    /**
     * Stop all threads in the target, optionally
     * passing in the thread that received an event.
     */
    virtual bool stop_all_threads(Thread*);

    virtual void stop_async();

    virtual size_t resume_all_threads();

    virtual size_t cleanup(Thread&);

    virtual Thread* exec(const ExecArg&, const char* const* env);

    virtual RefPtr<ProcessImpl>
        new_process(pid_t, const ExecArg*, ProcessOrigin);

    virtual RefPtr<SymbolMap> read_symbols();

    virtual addr_t setup_caller_frame(Thread&, addr_t, long);

    virtual void set_program_count(Thread&, addr_t);
    virtual void set_stack_pointer(Thread&, addr_t);

    virtual void set_result(Thread&, word_t);
    virtual void set_result64(Thread&, int64_t);
    virtual void set_result_double(Thread&, long double, size_t);

    virtual void read_environment(SArray&) const;
    virtual const std::string& command_line() const;

   /**
    *  @todo: throw logic_error? any better ideas?
    */
    virtual bool read_state(const Thread&, RunnableState&) const
    { return false; }

    virtual std::auto_ptr<DebugRegsBase> get_debug_regs(Thread&) const;

    virtual std::string thread_name(pid_t) const;

protected:
    const ELF::CoreFile& core() const;

    virtual ZObject* regs(const Thread&) const;

    virtual ZObject* fpu_regs(const Thread&) const;

    virtual bool write_register(Register&, const Variant&);

    virtual pid_t get_signal_sender_pid(const Thread&) const
    { return -1; } // unknown

    VirtualDSO* read_virtual_dso() const;

private:
    CorePtr core_;
    mutable Kind kind_;
};

#endif // LINUX_CORE_H__05A17E5D_58A4_11DA_B82B_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
