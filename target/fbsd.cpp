// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "dbgout.h"
#include "fbsd.h"

using namespace eventlog;
using namespace std;


////////////////////////////////////////////////////////////////
FreeBSDTarget::FreeBSDTarget(debugger_type& dbg)
    : BSDTarget(dbg)
{
}


////////////////////////////////////////////////////////////////
FreeBSDTarget::~FreeBSDTarget() throw()
{
}


////////////////////////////////////////////////////////////////
size_t FreeBSDTarget::enum_threads(EnumCallback<Thread*>* callback)
{
    if (callback)
    {
        ThreadMap::iterator i = threads_.begin();
        for (; i != threads_.end(); ++i)
        {
            callback->notify((*i).second.get());
        }
    }
    return threads_.size();
}

////////////////////////////////////////////////////////////////
Thread*
FreeBSDTarget::get_thread(pid_t pid, unsigned long id) const
{
    dbgout(0) << __func__ << "(" << pid << ", " << id << ")" << endl;
    if (pid == DEFAULT_THREAD)
    {
        // return first thread in the process
        return threads_.empty() ? 0 : threads_.begin()->second.get();
    }
    Thread* thread = NULL;

    if (id == unsigned(-1))
    {
        ThreadMap::const_iterator i = threads_.begin();
        for (; i != threads_.end(); ++i)
        {
            if (i->second->lwpid() == pid)
            {
                thread = i->second.get();
                break;
            }
        }
    }
    else
    {
        ThreadMap::const_iterator i = threads_.find(id);
        thread = i == threads_.end() ? 0 : i->second.get();
    }
    dbgout(0) << __func__ << "=" << thread << endl;

    return thread;
}


long FreeBSDTarget::syscall_num(const Thread& thread) const
{
    return 0; // todo
}


FreeBSDTarget::iterator FreeBSDTarget::threads_begin()
{
    return iterator(*this, threads_.begin());
}


FreeBSDTarget::iterator FreeBSDTarget::threads_end()
{
    return iterator(*this, threads_.end());
}


void FreeBSDTarget::add_thread_internal(const RefPtr<Thread>& thread)
{
    dbgout(0) << __func__ << ": " << thread->thread_id() << endl;
    assert(threads_.find(thread->thread_id()) == threads_.end());
    threads_[thread->thread_id()] = thread;
}


bool
FreeBSDTarget::remove_thread_internal(const RefPtr<Thread>& thread)
{
    const unsigned long tid = thread->thread_id();

    ThreadMap::iterator i = threads_.find(tid);

    if ((i != threads_.end()) && (thread == i->second))
    {
        threads_.erase(i);
        return true;
    }
    return false;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
