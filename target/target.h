#ifndef TARGET_H__F7939CAB_589A_11DA_B82B_00C04F09BBCC
#define TARGET_H__F7939CAB_589A_11DA_B82B_00C04F09BBCC
//
// $Id: target.h 720 2010-10-28 06:37:54Z root $
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
#include "zdk/enum.h"
#include "zdk/zerofwd.h"
#include "zdk/memio.h"
#include "zdk/shared_string.h"
#include "zdk/ref_ptr.h"
#include "zdk/register.h"
#include "debuggerfwd.h"
#include "generic/visitor.h"
#include "engine/debug_regs_base.h"
#include "engine/runnable_state.h"
#include <memory>
#include <string>
#include <vector>


class DebugChannel;
class ExecArg;
class ProcessImpl;
class SArray;

struct Target;
typedef RefPtr<Target> TargetPtr;


using Platform::reg_t;


/**
 * Models a debug target
 */
struct Target : public MemoryIO, public Visitable<bool>
{
    enum Kind
    {
        K_UNKNOWN,
        K_NATIVE_32BIT,
        K_NATIVE_64BIT,
        K_REMOTE_PROXY_32BIT,
        K_REMOTE_PROXY_64BIT,
    };
    // convenience wrappers around TargetFactory
    static TargetPtr new_core_target(debugger_type&, const char* = "");
    static TargetPtr new_live_target(debugger_type&, const char* = "");

    DebugChannel debug_channel(const char* fn) const;

    virtual ~Target() { }

    const std::string& param() const { return param_; }

    virtual void init(const char* param)
    { if (param) param_.assign(param); }

    virtual Kind kind() const = 0;

    virtual debugger_type& debugger() const = 0;

    // <machine-dependent>
    virtual word_t get_breakpoint_opcode(word_t) const = 0;

    /**
     * Reserve a "red-zone" on platforms that require it.
     * A red-zone is an area within the stack frame where local variables
     * (which do not need to be preserved across function calls) may be
     * allocated.
     */
    virtual addr_t stack_reserve(Thread&, addr_t) const = 0;
    virtual addr_t stack_align(Thread&, addr_t) const = 0;
    // </machine-dependent>

    virtual ProcessImpl* process() const = 0;
    virtual SymbolMap* symbols() const = 0;

    /**
     * @return the number of the current system call, or -1
     */
    virtual long syscall_num(const Thread&) const = 0;

    /**
     * @return the most recent function call result (that can
     * fit in one machine word, or one register); for example,
     * on the i386, return the contents of %EAX
     */
    virtual word_t result(const Thread&) const = 0;
    virtual void set_result(Thread&, word_t) = 0;

    virtual int64_t result64(const Thread&) const = 0;
    virtual void set_result64(Thread&, int64_t) = 0;

    virtual long double result_double(const Thread&, size_t) const = 0;
    virtual void set_result_double(Thread&, long double, size_t) = 0;

    virtual ZObject* regs(const Thread&) const = 0;
    virtual ZObject* fpu_regs(const Thread&) const = 0;

    /**
     * Enumerate the general purpose registers, return number of
     * registers. If a non-null pointer to a callback object is given,
     * then call its notify() method for each register.
     */
    virtual size_t enum_user_regs(
        const Thread&,
        EnumCallback<Register*>*) const = 0;

    /**
     * Enumerate the FPU (and possible extended) registers, in the
     * same manner as enum_user_regs
     */
    virtual size_t enum_fpu_regs(
        const Thread&,
        EnumCallback<Register*>*) const = 0;

    /**
     * Modify the n-th general purpose CPU register.
     * Which register is actually modified is platform-specific
     */
    virtual void write_register(Thread&, size_t n, reg_t) = 0;

    /**
     * This is a generalized method for modifying the contents
     * of a register in the debugged program, works for general
     * purpose registers as well as for special purpose registers
     * (such as FPU, etc).
     */
    virtual bool write_register(Register&, const Variant&) = 0;

    virtual void set_registers(Thread&, ZObject*, ZObject*) = 0;

    /**
     * Read the contents of the n-th general purpose register,
     * optionally fetching it from the active's frame saved state.
     */
    virtual bool read_register( const Thread&,
                                int regNum,
                                bool fromStackFrame,
                                reg_t&) const = 0;

    // methods for retrieving special registers

    virtual addr_t program_count(const Thread&) const = 0;
    virtual addr_t frame_pointer(const Thread&) const = 0;
    virtual addr_t stack_pointer(const Thread&) const = 0;

    /**
     * Force execution to resume at given address,
     * by explicitly modifying the program counter
     */
    virtual void set_program_count(Thread&, addr_t) = 0;

    virtual void set_stack_pointer(Thread&, addr_t) = 0;

    /**
     * Called when setting up the stack frame for calling
     * a function from within the expression interpreter.
     */
    virtual addr_t setup_caller_frame(Thread&, addr_t sp, long pc) = 0;

    /**
     * @return true if the dynamic linker/loader generates
     * events and we have successfully installed a breakpoint
     * to intercept them.
     */
    virtual bool has_linker_events() const = 0;

    /**
     * Attach to running process
     */
    virtual void attach(pid_t) = 0;

    virtual void detach(bool no_throw = false) = 0;

    /**
     * Execute a command (with optional environment) and
     * return the main thread of the newly spawned process
     */
    virtual Thread* exec(const ExecArg&, const char* const* env) = 0;

    virtual Thread* get_thread(pid_t, unsigned long thread_id = 0) const = 0;

    /**
     * thread ID to light-weight process ID
     */
    virtual pid_t tid_to_lwpid(long) const = 0;

    virtual unsigned long lwpid_to_tid(pid_t) const = 0;

    /**
     * map PID to Thread in which event has occurred
     * (may internally require a PID to LWPID translation).
     */
    virtual Thread* event_pid_to_thread(pid_t) const = 0;

    /**
     * Enumerate all the attached threads, return thread count.
     */
    virtual size_t enum_threads(EnumCallback<Thread*>* = NULL) = 0;

    virtual bool is_attached(Thread* = NULL) const = 0;

    /**
     * Stop all threads in the target, optionally
     * passing in the thread that received an event.
     */
    virtual bool stop_all_threads(Thread* = NULL) = 0;

    /**
     * Tell all threads to stop, but don't wait.
     */
    virtual void stop_async() = 0;

    /**
     * @return number of actual resumed threads.
     */
    virtual size_t resume_all_threads() = 0;

    /**
     * Called when a thread finishes execution.
     * @return the remaining number of running threads in this target
     */
    virtual size_t cleanup(Thread&) = 0;

    /**
     * @return true when the handling of an event requires all
     * threads in the target to be stopped by the debugger
     * @note the purpose of the method is to optimize handling
     * of events such as creation of new threads.
     */
    virtual bool event_requires_stop(Thread*) = 0;

    virtual void handle_event(Thread*) = 0;

    /**
     * @return the symbol tables that correspond to the
     * linux-gate.so.1 virtual dynamic shared object.
     */
    virtual RefPtr<SymbolTable> vdso_symbol_tables() const = 0;

    /**
     * @return the address where a vDSO is mapped
     */
    virtual addr_t vdso_addr(size_t* size = NULL) const = 0;

    virtual TypeSystem* type_system() const = 0;

    virtual void read_environment(SArray&) const = 0;

    virtual bool read_state(const Thread&, RunnableState&) const = 0;

    /**
     * @return the command line that started the
     * traced process
     * @note for core dumps, returns the name of core file
     */
    virtual const std::string& command_line() const = 0;

    /**
     * For use in the expression interpreter.
     * Pass as many parameters as possible into registers,
     * if the architecture allows it. The parameters that are
     * copied to cpu registers are removed from the input vector.
     * @return true if any registers were used
     */
    virtual bool pass_by_reg(Thread&, std::vector<RefPtr<Variant> >&) = 0;

    virtual pid_t get_signal_sender_pid(const Thread&) const = 0;

    virtual std::auto_ptr<DebugRegsBase> get_debug_regs(Thread&) const = 0;

    /**
     * Handle the case when the breakpoint opcode is smaller
     * than a machine word
     */
    virtual void step_until_safe(Thread&, addr_t) const { }

    virtual bool map_path(std::string&) const { return false; }
    virtual RefPtr<SharedString> process_name(pid_t = 0) const;

    virtual const std::string& procfs_root() const;

    // support re-mapping filenames
    virtual std::auto_ptr<std::istream> get_ifstream(const char*) const;

    virtual const char* id() const;

    virtual std::string thread_name(pid_t) const;

    virtual void update_threads_info() { }

    ////////////////////////////////////////////////////////////
    //
    // Memory read / write methods
    //
    enum SegmentType { DATA_SEGMENT, CODE_SEGMENT };

    virtual void read_memory(
        pid_t           pid,
        SegmentType     segType,
        addr_t          address,
        word_t*         buffer,
        size_t          howManyWords,
        size_t*         wordsRead = 0) const = 0;

    virtual void write_memory(
        pid_t           pid,
        SegmentType     seg,
        addr_t          addr,
        const word_t*   buf,
        size_t          wordsToWrite) = 0;

protected:
    void reset_process_name() 
    {
        processName_.reset();
    }

private:
    mutable std::string procfs_;
    mutable RefPtr<SharedString> processName_;
    std::string param_;
};

#endif // TARGET_H__F7939CAB_589A_11DA_B82B_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
