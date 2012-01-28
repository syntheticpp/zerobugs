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
#include "syscall_pending.h"
#include "target.h"
#include "zdk/zero.h"
#include "zdk/zobject_scope.h"

using namespace std;


#ifdef __linux__
bool thread_syscall_pending(const Thread& thread, reg_t pc)
{
    if (pc == 0)
    {
        pc = thread.program_count();
    }

    // special address used by the expression interpreter?
    if (pc == INVALID_PC_ADDR)
    {
        return false;
    }
    bool syscallPending = false;
    ZObjectScope scope;

#if (__i386__)
 /*
    In older kernels EIP may sometimes be 0xFFFFE002.
    It's up in kernel space, if I try to read
    the memory with PTRACE_PEEKDATA, the call fails
    with EIO -- this happens when the thread is doing a
    system call:
     int $0x80
     ret
    or
     sysenter
     ret
 */
    if (pc == 0xFFFFE002)
    {
        syscallPending = true;
    }
    else
#endif
        if (Process* proc = thread.process())
    {
        RefPtr<Symbol> sym(thread.symbols()->lookup_symbol(pc));
        if (sym && sym->table(&scope) == proc->vdso_symbol_tables())
        {
            syscallPending = true;
        }
#if (__i386__) || (__x86_64__)
        else
        {
            word_t opcode = 0;
            size_t wordsRead = 0;
            thread.read_code(pc - 2, &opcode, 1, &wordsRead);

            if ((opcode & 0xffff) == 0x80cd     // int 0x80
              ||(opcode & 0xffff) == 0x050f)    // sysenter
            {
                syscallPending = true;
            }

            // hack around in the event that vdso_symbol_tables_failed
            // (it happens on 2.6.17 x86_64 when running 32-bit apps)
            if (!syscallPending && thread.is_32_bit())
            {
            #if 0
                // may produce some false positives
                if (Target* target = thread.target())
                {
                    size_t size = 0;
                    addr_t vDSOAddr = target->vdso_addr(&size);

                    if (vDSOAddr <= pc && (pc - vDSOAddr) < size)
                    {
                    #ifdef DEBUG
                        clog << __func__ << ": " << (void*)pc << endl;
                    #endif
                        syscallPending = true;
                    }
                }
            #else
                syscallPending = (pc == 0xffffe405);
            #endif
            }
        }
#endif
    }
    return syscallPending;
}


/**
 * @note will fail when called on a core file
 */
void thread_cancel_syscall(Thread& thread)
{
    if (Target* target = thread.target())
    {
        // set EAX/RAX to zero; on Linux this has
        // the effect of cancelling the system call
        //
        // question: how does it work on the PPC?
        target->set_result(thread, 0);
    }

}

#else
//
// generic place holders
//
bool thread_syscall_pending(const Thread&, reg_t)
{
    return false;
}


void thread_cancel_syscall(Thread&)
{
}
#endif
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
