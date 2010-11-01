#ifndef FBSD_CORE_H__D77AF1D5_5AE6_11DA_8F2D_00C04F09BBCC
#define FBSD_CORE_H__D77AF1D5_5AE6_11DA_8F2D_00C04F09BBCC
//
// $Id: fbsd_core.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "fbsd.h"
#include "zdk/check_ptr.h"
#include "corefwd.h"


class FreeBSDCoreTarget : public FreeBSDTarget
{
    DECLARE_VISITABLE()

public:
    explicit FreeBSDCoreTarget(debugger_type&);

    ~FreeBSDCoreTarget() throw ();

    // MemoryIO
    virtual void read_memory(
        pid_t       lwpid,
        SegmentType segType, // code or data
        addr_t      address,
        word_t*     buffer,
        size_t      howManyWords,
        size_t*     wordsRead = 0) const;

    virtual void write_memory(
        pid_t       lwpid,
        SegmentType segType, // code or data
        addr_t      addr,
        const word_t* sourceBuf,
        size_t      wordsToWrite);

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

    virtual word_t get_breakpoint_opcode(word_t) const;

    virtual Thread* exec(const ExecArg&, const char* const* env);

    virtual RefPtr<SymbolMap> read_symbols();

    virtual void set_program_count(Thread&, addr_t);
    virtual void set_stack_pointer(Thread&, addr_t);

    virtual void set_result(Thread&, word_t);
    virtual void set_result64(Thread&, int64_t);
    virtual void set_result_double(Thread&, long double, size_t);
    virtual void set_registers(Thread&, ZObject*, ZObject*);

protected:
    const ELF::CoreFile& core() const { return *CHKPTR(core_); }

    virtual ZObject* regs(const Thread&) const;

    virtual ZObject* fpu_regs(const Thread&) const;

    virtual void write_register(Thread&, size_t, reg_t);
    virtual bool write_register(Register&, const Variant&);
    virtual bool read_register( const Thread&,
                                int regnum,
                                bool fromStackFrame,
                                reg_t&) const;
    virtual void read_environment(SArray&) const;

    RefPtr<ProcessImpl> new_process(pid_t, const ExecArg*, ProcessOrigin);

    virtual unsigned long lwpid_to_tid(pid_t) const;
    virtual bool read_state(const Thread&, RunnableState&) const;
    virtual pid_t get_signal_sender_pid(const Thread&) const;

    virtual addr_t setup_caller_frame(Thread&, addr_t sp, long pc);
    virtual std::auto_ptr<DebugRegsBase> get_debug_regs(Thread&) const;

private:
    CorePtr core_;
};

#endif // FBSD_CORE_H__D77AF1D5_5AE6_11DA_8F2D_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
