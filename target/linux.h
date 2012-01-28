#ifndef LINUX_TARGET_H__F689B353_589F_11DA_B82B_00C04F09BBCC
#define LINUX_TARGET_H__F689B353_589F_11DA_B82B_00C04F09BBCC
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
#include "zdk/arch-linux.h"
#include <sys/param.h>
#include <sys/user.h>
#include <map>
#include <vector>
#include "target/cpu.h"
#include "target/iterator.h"
#include "target/reg.h"
#include "target/unix.h"

#ifndef AT_SYSINFO_EHDR
 #define AT_SYSINFO_EHDR 33
#endif

// virtual dynamic shared object, in newer kernels
class VirtualDSO;

// general purpose registers
typedef Regs<user_regs_struct> GRegs;

// FP and extended registers
typedef FpuRegs<user_fpxregs_struct> FPXRegs;
typedef FpuRegs<user_fpxregs_32> FPXRegs32;

typedef std::vector<std::pair<word_t, word_t> > AuxVect;


bool get_sysinfo_ehdr(const AuxVect&, addr_t&, size_t& pageSize);


template<typename T>
ZDK_LOCAL
inline RefPtr<Register> make_fpxreg(const char*     name,
                                    const Thread&   thread,
                                    size_t          offset,
                                    T               value)
{
    return new Reg<T, REG_FPUX>(name, thread, offset, value);
}


/**
 * Base for live and corefile linux targets
 */
class LinuxTarget : public UnixTarget
{
    // non-copyable, non-assignable
    LinuxTarget(const LinuxTarget&);
    LinuxTarget& operator=(const LinuxTarget&);

protected:
    explicit LinuxTarget(debugger_type&);

    ~LinuxTarget() throw();

    virtual RefPtr<SymbolTable> vdso_symbol_tables() const;
    virtual addr_t vdso_addr(size_t*) const;

    /**
     * @return a pointer to an object that models the
     * linux-gate.so.1 virual shared object, exposed by
     * newer kernels.
     * @return NULL if there's an error detecting the address
     * of the VDSO, or the kernel does not support it.
     */
    virtual VirtualDSO* read_virtual_dso() const = 0;

   /**
    * @note non-virtual, for internal use
    */
    Thread* get_thread(pid_t lwpid) const;

public:
    // in Linux, each thread corresponds to
    // a unique LWP (lightweight process)
    typedef std::map<pid_t, RefPtr<Thread> > ThreadMap;

    typedef RefCountedIterator<
        Thread,
        LinuxTarget,
        LinuxTarget::ThreadMap> iterator;

    friend class RefCountedIterator<
        Thread,
        LinuxTarget,
        LinuxTarget::ThreadMap>;

    long syscall_num(const Thread& thread) const;

    bool read_register(const Thread&, int, bool, reg_t&) const;

    /* NOTE: Implement here both getters and setters so
       that they are conveniently kept together. Derived
       classes (such as Core target implementations)
       need to override the setters to ensure read-only
       behavior, where necessary.
     */
    word_t result(const Thread&) const;
    void set_result(Thread&, word_t);

    int64_t result64(const Thread&) const;
    void set_result64(Thread&, int64_t);

    long double result_double(const Thread&, size_t) const;
    void set_result_double(Thread&, long double, size_t);

    void set_program_count(Thread&, addr_t);

    addr_t program_count(const Thread& thread) const;

    addr_t frame_pointer(const Thread& thread) const;

    void set_stack_pointer(Thread&, addr_t);
    addr_t stack_pointer(const Thread& thread) const;

    void set_registers(Thread&, ZObject*, ZObject*);

    /**
     * Enumerate the general purpose registers, return number of
     * registers. If a non-null pointer to a callback object is given,
     * then call its notify() method for each register.
     */
    size_t enum_user_regs(const Thread&, EnumCallback<Register*>*) const;

    /**
     * Enumerate the FPU and extended registers (e.q. XMM, SSE)
     */
    size_t enum_fpu_regs(const Thread& thread, EnumCallback<Register*>*) const;

    /**
     * close all the files that are open in the debugger
     * process -- used when forking and execing.
     */
    void close_all_files();

    virtual size_t enum_threads(EnumCallback<Thread*>* = NULL);

    virtual Thread* get_thread(pid_t, unsigned long) const;

    /**
     * map PID to Thread in which event has occurred
     */
    virtual Thread* event_pid_to_thread(pid_t) const;

    /**
     * thread ID to light-weight process ID
     */
    virtual pid_t tid_to_lwpid(long) const;
    virtual unsigned long lwpid_to_tid(pid_t) const { return 0; }

    iterator threads_begin();
    iterator threads_end();

    bool empty() const { return threads_.empty(); }

    size_t size() const { return threads_.size(); }

    ThreadMap threads() const { return threads_; }

private:
    void add_thread_internal(const RefPtr<Thread>&);
    bool remove_thread_internal(const RefPtr<Thread>&);

private:
    ThreadMap threads_;

    mutable VirtualDSO* vdso_;
    mutable RefPtr<SymbolTable> vdsoTables_;
};


#endif // LINUX_TARGET_H__F689B353_589F_11DA_B82B_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
