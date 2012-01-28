#ifndef FBSD_H__9012645A_5AF0_11DA_8F2D_00C04F09BBCC
#define FBSD_H__9012645A_5AF0_11DA_8F2D_00C04F09BBCC
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
extern "C"
{
  #include <thread_db.h>  // for thread_t
}
#include <map>
#include "target/cpu.h"
#include "target/bsd.h"
#include "target/iterator.h"

// general purpose registers
typedef Regs<user_regs_struct> GRegs;


class FreeBSDTarget : public BSDTarget
{
protected:
    // because the mapping between user-space threads
    // and kernel threads is dynamic, we refer to threads
    // by their thread ID rather than lwpid_t
    //
    // TODO: consider alternative structures, such as hash_map
    // or google sparse_hash_map
    //
    typedef std::map<thread_t, RefPtr<Thread> > ThreadMap;

    typedef RefCountedIterator<
        Thread,
        FreeBSDTarget,
        FreeBSDTarget::ThreadMap> iterator;

    friend class RefCountedIterator<
        Thread,
        FreeBSDTarget,
        FreeBSDTarget::ThreadMap>;

    explicit FreeBSDTarget(debugger_type&);

    ~FreeBSDTarget() throw();

    Kind kind() const;

    virtual RefPtr<SymbolTable> vdso_symbol_tables() const
    { return RefPtr<SymbolTable>(); }

    virtual addr_t vdso_addr(size_t*) const  { return 0; }

public:
    virtual size_t enum_threads(EnumCallback<Thread*>* = NULL);

    long syscall_num(const Thread& thread) const;

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
    size_t enum_fpu_regs(const Thread&, EnumCallback<Register*>*) const;

    // threads

    virtual Thread* get_thread(pid_t, unsigned long) const;

    /**
     * map PID to Thread in which event has occurred
     */
    virtual Thread* event_pid_to_thread(pid_t) const;

    /**
     * thread ID to light-weight process ID
     */
    virtual pid_t tid_to_lwpid(long) const;

    iterator threads_begin();
    iterator threads_end();

    bool empty() const { return threads_.empty(); }
    size_t size() const { return threads_.size(); }

private:
    void add_thread_internal(const RefPtr<Thread>&);
    bool remove_thread_internal(const RefPtr<Thread>&);

    virtual addr_t stack_reserve(Thread&, addr_t addr) const
    {
        return addr;
    }
    virtual addr_t stack_align(Thread&, addr_t addr) const
    {
        return addr;
    }

    const std::string& command_line() const
    {
        return commandLine_; // todo
    }

protected:
    std::string commandLine_;

private:
    ThreadMap threads_;
};

#endif // FBSD_H__9012645A_5AF0_11DA_8F2D_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
