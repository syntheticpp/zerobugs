//
// $Id: linux.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "dbgout.h"
#include "debugger_base.h"
#include "target/linux.h"
#include "zdk/shared_string_impl.h"
#include "zdk/zero.h"
#include "dharma/directory.h"
#include "dharma/syscall_wrap.h"
#include "dharma/virtual_dso.h"
#include "engine/thread.h"
#include "symbolz/private/symbol_table_impl.h"

using namespace std;
using namespace eventlog;


bool get_sysinfo_ehdr(const AuxVect& v, addr_t& addr, size_t& sz)
{
    assert(addr == 0);
    assert(sz == 0);

    AuxVect::const_iterator i = v.begin();
    for (; i != v.end(); ++i)
    {
        if (i->first == AT_SYSINFO_EHDR)
        {
            addr = static_cast<addr_t>(i->second);

            if (sz)
            {
                break;
            }
        }
        else if (i->first == 6 /* AT_PAGESZ */)
        {
            sz += i->second;

            if (addr)
            {
                break;
            }
        }
    }
    return addr && sz;
}


LinuxTarget::LinuxTarget(debugger_type& debugger)
    : UnixTarget(debugger)
    , vdso_(0)
{
}


LinuxTarget::~LinuxTarget() throw()
{
    delete vdso_;
}


LinuxTarget::iterator LinuxTarget::threads_begin()
{
    return iterator(*this, threads_.begin());
}


LinuxTarget::iterator LinuxTarget::threads_end()
{
    return iterator(*this, threads_.end());
}


void LinuxTarget::add_thread_internal(const RefPtr<Thread>& thread)
{
    assert(threads_.find(thread->lwpid()) == threads_.end());
    threads_[thread->lwpid()] = thread;
}


bool
LinuxTarget::remove_thread_internal(const RefPtr<Thread>& thread)
{
    const pid_t pid = thread->lwpid();

    dbgout(0) << __func__ << ": " << pid << endl;
    ThreadMap::iterator i = threads_.find(pid);

    if ((i != threads_.end()) && (thread == i->second))
    {
        threads_.erase(i);
        return true;
    }
    return false;
}



size_t LinuxTarget::enum_threads(EnumCallback<Thread*>* callback)
{
    if (callback)
    {
        ThreadMap::iterator i = threads_.begin();
        for (; i != threads_.end(); ++i)
        {
            if (!is_deletion_pending(i->second))
            {
                callback->notify((*i).second.get());
            }
        }
    }
    return threads_.size();
}


void LinuxTarget::close_all_files()
{
    try
    {
        Directory fd(procfs_root() + "self/fd");

        Directory::const_iterator i = fd.begin();
        for (; i != fd.end(); ++i)
        {
            const int f = atoi(i.short_path().c_str());
            if (f > 2)
            {
                close(f);
            }
        }
    }
    catch (const exception& e)
    {
        cerr << __func__ << ": " << e.what() << endl;
    }
}



Thread* LinuxTarget::get_thread(pid_t pid) const
{
    ThreadMap::const_iterator i = threads_.find(pid);
    if (i == threads_.end())
    {
        return NULL;
    }
    if (is_deletion_pending(i->second))
    {
        return NULL;
    }
    return i->second.get();
}


Thread* LinuxTarget::get_thread(pid_t lwpid, unsigned long) const
{
    if (lwpid == DEFAULT_THREAD)
    {
        // return first thread in the process
        return threads_.empty() ? 0 : threads_.begin()->second.get();
    }
    return get_thread(lwpid);
}


Thread* LinuxTarget::event_pid_to_thread(pid_t pid) const
{
    assert(pid);

    Thread* thread = get_thread(pid);
    dbgout(1) << __func__ << "(" << pid << ")=" << thread << endl;
    return thread;
}


pid_t LinuxTarget::tid_to_lwpid(long tid) const
{
    return 0;
}


void
LinuxTarget::set_registers(Thread& thread, ZObject* usr, ZObject* fpu)
{
    if (usr)
    {
       GRegs& regs = interface_cast<GRegs&>(*usr);
       // todo: sys::set_regs
       sys::ptrace(__ptrace_request(PTRACE_SETREGS),
                    thread.lwpid(),
                    0,
                    (long)static_cast<user_regs_struct*>(&regs));
    }
    if (fpu)
    {
        FPXRegs& regs = interface_cast<FPXRegs&>(*fpu);
        sys::set_regs(thread.lwpid(),
                      static_cast<user_fpxregs_struct&>(regs));
    }
}


RefPtr<SymbolTable> LinuxTarget::vdso_symbol_tables() const
{
    if (!vdso_)
    {
        vdso_ = read_virtual_dso();
    }
    if (!vdsoTables_ && vdso_)
    {
        // It would be cute to use "linux-gate.so.1" here
        // instead of the process' name; unfortunatelly the disassembler
        // and other parts of the debugger may want to read the file.

        const char* filename = process()->name();

        RefPtr<SymbolTableImpl> symTabList = SymbolTableImpl::read_tables(
            process(),
            shared_string(filename),
            vdso_->binary(),
            *debugger().symbol_table_events(),
            vdso_->addr(),
            vdso_->addr() + vdso_->size(),
            false); // don't call SymbolEvents::on_done(),
                    // so that we do not attempt to restore
                    // Module breakpoints and settings for
                    // the VDSO (alternately, we could check
                    // inside SymbolEvents, but this is easier).

        if (symTabList)
        {
            symTabList->set_is_virtual_shared_object();
            vdsoTables_ = symTabList;
        }
    }
    return vdsoTables_;
}


addr_t LinuxTarget::vdso_addr(size_t* size) const
{
    if (!vdso_)
    {
        vdso_ = read_virtual_dso();
    }
    if (size)
    {
        *size = vdso_->size();
    }
    return vdso_->addr();

}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
