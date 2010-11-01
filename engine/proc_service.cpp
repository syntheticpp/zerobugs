//
// $Id: proc_service.cpp 729 2010-10-31 07:00:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// Implement functions expected by thread_db.
//
#include "zdk/config.h"
#include "target/config.h"
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#ifdef HAVE_ASM_LDT_H
 #include <asm/ldt.h>
#endif

#ifdef __x86_64__
#include <sys/ptrace.h>
#ifdef HAVE_ASM_PRCTL_H
 #include <asm/prctl.h>
#endif
#ifdef HAVE_ASM_PTRACE_H
 #include <asm/ptrace.h>
#endif
#endif

#ifdef HAVE_ASM_UNISTD_H
 #include <asm/unistd.h>
#endif
#include <thread_db.h>
#include <iostream>
#include <vector>
#include "zdk/align.h"
#include "zdk/check_ptr.h"
#include "zdk/zero.h"
#include "zdk/zobject_scope.h"
#include "dharma/environ.h"
#include "dharma/symbol_util.h"
#include "dharma/syscall_wrap.h"

#ifdef HAVE_PROC_SERVICE_H
 #include <proc_service.h>
#else
 typedef td_err_e ps_err_e;
#ifndef PS_ERR
 #define PS_ERR TD_ERR
#endif

#ifndef PS_OK
 #define PS_OK TD_OK
#endif
#endif

#ifndef PTRACE_GET_THREAD_AREA
 #define PTRACE_GET_THREAD_AREA 25
#endif


using namespace std;
using Platform::addr_t;


extern "C"
{ // C-linkage expected by thread_db


ps_err_e ps_pstop(ps_prochandle*)
{
#ifdef DEBUG
    clog << __func__ << endl;
#endif
    return PS_ERR;
}


ps_err_e ps_pcontinue(ps_prochandle*)
{
#ifdef DEBUG
    clog << __func__ << endl;
#endif
    return PS_ERR;
}


ps_err_e ps_lstop(ps_prochandle* /* proc */, pid_t /* lwpid */)
{
#ifdef DEBUG
    clog << __func__ << endl;
#endif
    return PS_ERR;
}


ps_err_e ps_lcontinue(ps_prochandle* /* proc */, pid_t /* lwpid */)
{
#ifdef DEBUG
    clog << __func__ << endl;
#endif
    return PS_ERR;
}


/**
 * Search for the symbol named name within the object named OBJ within
 * the target process proc.  If the symbol is found the address of the
 * symbol is stored in addr.
 */
ps_err_e ps_pglobal_lookup(ps_prochandle*  proc,
                           const char*     obj,
                           const char*     name,
                           psaddr_t*       addr)
{
    assert(addr);
    *addr = 0;

    try
    {
        SymbolEnum e;

        SymbolMap* symbols = proc ? proc->symbols() : NULL;
        if (!symbols)
        {
            return PS_ERR; // symbols not loaded
        }
        symbols->enum_symbols(name, &e, (SymbolTable::LKUP_DYNAMIC | SymbolTable::LKUP_ISMANGLED));
        if (e.size())
        {
            // hack: cache the symbol table where the name was
            // found -- it is very likely the other thread_db-related
            // symbols are in the same table
            if (!proc->symtab_)
            {
                ZObjectScope scope;
                proc->symtab_ = e.front()->table(&scope);
            }
            assert(addr);
            // return the first match
            *addr = psaddr_t(e.front()->addr());
            return PS_OK;
        }
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
    }
    return PS_ERR;
}


/**
 * Read SIZE bytes from the target process at address ADDR and copy
 * them into BUF.
 */
ps_err_e
ps_pdread(ps_prochandle* proc, psaddr_t psaddr, void* buf, size_t size)
{
    assert(proc);
    assert(buf);

    if (psaddr)
    {
        // compute the size in machine-words
        size_t count = round_to_word(size);
        vector<word_t> tmp(count);

        try
        {
            const addr_t addr = (addr_t)psaddr;
            size_t wordsRead = 0;
            proc->read_data(addr, &tmp[0], count, &wordsRead);

            memcpy(buf, &tmp[0], size);
            return PS_OK;
        }
        catch (const exception& e)
        {
            cerr << __func__ << ": " << e.what() << endl;
        }
    }
    return PS_ERR;
}


/**
 * Write SIZE bytes from BUF into the target process at address ADDR.
 */
ps_err_e ps_pdwrite(ps_prochandle* proc,
                    psaddr_t psaddr,
                    const void* buf,
                    size_t size)
{
    assert(proc);
    assert(buf);

    assert(psaddr);

    try
    {
        const addr_t addr = (addr_t)psaddr;

        // exact number of machine words?
        if (size % sizeof(word_t) == 0)
        {
            proc->write_data(
                addr,
                reinterpret_cast<const word_t*>(buf),
                size/sizeof(word_t));
        }
        else
        {
            size_t count = round_to_word(size);
            vector<word_t> tmp(count);

            proc->read_data(addr, &tmp[0], count);
            memcpy(&tmp[0], buf, size);

            proc->write_data(addr, &tmp[0], count);
        }
        return PS_OK;
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
    }
    return PS_ERR;
}


ps_err_e
ps_pread(ps_prochandle* proc, psaddr_t addr, void* buf, size_t size)
{
    return ps_pdread(proc, addr, buf, size);
}


ps_err_e
ps_pwrite(ps_prochandle* proc,
          psaddr_t addr,
          const void* buf,
          size_t size)
{
    return ps_pdwrite(proc, addr, buf, size);
}


/**
 * Get the general registers of lwpid within the target process proc
 * and store them in gregset.
 */
ps_err_e
ps_lgetregs(ps_prochandle* proc, lwpid_t lwpid, prgregset_t gregset)
{
    assert(proc);

    user_regs_struct regs;
    memset(&regs, 0, sizeof regs);

    try
    {
        sys::get_regs(lwpid, regs);
        *reinterpret_cast<user_regs_struct*>(gregset) = regs;
        return PS_OK;
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
    }

    return PS_ERR;
}

/**
 * Set the general registers of LWPID within the target process proc
 * from GREGSET.
 */
ps_err_e
ps_lsetregs(ps_prochandle* proc, lwpid_t lwpid, const prgregset_t)
{
#ifdef DEBUG
    clog << __func__ << endl;
#endif
    return PS_ERR;
}

/**
 * Get the floating-point registers of LWPID within the target
 * process proc and store them in FPREGSET.
 */
ps_err_e
ps_lgetfpregs(ps_prochandle* proc, pid_t, prfpregset_t* fpregset)
{
#ifdef DEBUG
    clog << __func__ << endl;
#endif
    return PS_ERR;
}


/**
 * Set the floating-point registers of LWPID within the target
 * process PROC from FPREGSET.
 */
ps_err_e
ps_lsetfpregs(ps_prochandle* proc, pid_t, const prfpregset_t*)
{
#ifdef DEBUG
    clog << __func__ << endl;
#endif
    return PS_ERR;
}


/**
 * Return overall process id of the target
 */
pid_t ps_getpid (ps_prochandle* target)
{
    pid_t pid = 0;

    if (target)
    {
        if (Process* proc = target->process())
        {
            pid = proc->pid();
        }
    }
    return pid;
}


#if defined(__NR_get_thread_area) && defined(_syscall1)
 _syscall1(int, get_thread_area, modify_ldt_t*, uinfo);
#endif

#ifdef __FreeBSD__
ps_err_e
ps_get_thread_area(ps_prochandle* target, pid_t pid, int idx, addr_t* addr)
{
    // todo
    return PS_ERR;
}
#elif defined(__x86_64__)
ps_err_e
ps_get_thread_area(ps_prochandle* target, pid_t pid, int idx, addr_t* addr)
{
    assert(target);

    try
    {
        switch (idx)
        {
        case 25:
            sys::ptrace(__ptrace_request(PTRACE_ARCH_PRCTL), pid,
                    word_t(addr), ARCH_GET_FS);
            return PS_OK;

        case 26:
            sys::ptrace(__ptrace_request(PTRACE_ARCH_PRCTL), pid,
                    word_t(addr), ARCH_GET_GS);
            return PS_OK;

        default:
            break;
        }
    }
    catch (const exception& e)
    {
        // cerr << __func__ << ": " << e.what() << endl;
    }
    return PS_ERR;
}
#else

////////////////////////////////////////////////////////////////
ps_err_e ps_get_thread_area(ps_prochandle* target,
                            pid_t pid,
                            int idx,
                            addr_t* addr)
{
#ifdef DEBUG
    clog << __func__ << ": pid=" << pid << ", idx=" << idx << "\n";
#endif
    assert(pid == ps_getpid(target));

    if (addr)
    {
        *addr = 0;
    }

#if defined(__NR_get_thread_area) && defined(_syscall1)
    // use system call, if available

    modify_ldt_t uinfo;
    uinfo.entry_number = idx + 1;

    if (get_thread_area(&uinfo) == 0)
    {
        *addr = uinfo.base_addr;
        return PS_OK;
    }
    else
#endif
    {
        // fallback to ptrace

        word_t buf[4];
        try
        {
            sys::ptrace((__ptrace_request)PTRACE_GET_THREAD_AREA,
                            pid,
                            idx,
                            reinterpret_cast<word_t>(&buf));
            *addr = buf[1];
            return PS_OK;
        }
        catch (const exception& e)
        {
            cerr << __func__ << ": " << e.what() << endl;
        }
    }
#ifdef DEBUG
    clog << __func__ << "=" << hex << *addr << dec << "\n";
#endif
    return PS_ERR;
}
#endif // __i386__

ps_err_e ps_linfo(struct ps_prochandle *, lwpid_t, void *)
{
    cerr << __func__ << ": not implemented\n";
    return PS_ERR;
}


////////////////////////////////////////////////////////////////
#if defined(__i386__) || defined(__x86_64__)
//
// XMM registers are Intel-specific
//
ps_err_e ps_lgetxmmregs(ps_prochandle*, lwpid_t, char*)
{
    cerr << __func__ << ": not implemented\n";
    return PS_ERR;
}


ps_err_e ps_lsetxmmregs(ps_prochandle*, lwpid_t, const char*)
{
    cerr << __func__ << ": not implemented\n";
    return PS_ERR;
}
#endif  // __i386__

} // extern "C"

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
