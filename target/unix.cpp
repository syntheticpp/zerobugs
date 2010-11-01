//
// $Id: unix.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "target/unix.h"
#include "debugger_base.h"
#include "zdk/thread_util.h"
#include "typez/public/native_type_system.h"

using namespace eventlog;


UnixTarget::UnixTarget(DebuggerBase& dbg)
    : dbg_(dbg)
    , wordSize_(__WORDSIZE)
    , types_(new NativeTypeSystem(wordSize_))
    , iterCount_(0)
{
}


UnixTarget::~UnixTarget() throw()
{
}


void UnixTarget::handle_event(Thread* thread)
{
    assert(thread);

    if (!thread_stopped(*thread))
    {
        assert(thread_finished(*thread));
    }

    debugger().on_event(*thread);
}


void
UnixTarget::init_process(pid_t pid,
                         const ExecArg* arg,
                         ProcessOrigin orig, // how started/attached?
                         const char* name)
{
    proc_ = new_process(pid, arg, orig);
    if (name)
    {
        // override the automatically-determined process name
        proc_->set_name(name);
    }
}


void UnixTarget::init_symbols(SymbolMap* symbols)
{
    if (symbols)
    {
        symbols_ = symbols;
    }
    else
    {
        symbols_ = read_symbols();
    }
    assert(symbols_); // post-condition
}


bool UnixTarget::is_attached(Thread* thread) const
{
    return thread
        ? threads_.find(thread) != threads_.end()
        : !threads_.empty();
}


int UnixTarget::verbose() const
{
    return debugger().verbose();
}


void UnixTarget::read_data( addr_t  addr,
                            word_t* buf,
                            size_t  count,
                            size_t* wordsRead
                          ) const
{
    if (Process* proc = process())
    {
        pid_t pid = proc->pid();
        read_memory(pid, DATA_SEGMENT, addr, buf, count, wordsRead);
    }
    else if (wordsRead)
    {
        *wordsRead = 0;
    }
}


void
UnixTarget::write_data(addr_t addr, const word_t* buf, size_t count)
{
    if (Process* proc = process())
    {
        pid_t pid = proc->pid();
        write_memory(pid, DATA_SEGMENT, addr, buf, count);
    }
}


void UnixTarget::read_code(
    addr_t  addr,
    word_t* buf,
    size_t  count,
    size_t* wordsRead) const
{
    if (Process* proc = process())
    {
        pid_t pid = proc->pid();
        read_memory(pid, CODE_SEGMENT, addr, buf, count, wordsRead);
    }
    else if (wordsRead)
    {
        *wordsRead = 0;
    }
}


void
UnixTarget::write_code(addr_t addr, const word_t* buf, size_t count)
{
    if (Process* proc = process())
    {
        pid_t pid = proc->pid();
        write_memory(pid, CODE_SEGMENT, addr, buf, count);
    }
}


void UnixTarget::manage_thread(const RefPtr<Thread>& thread)
{
    threads_.insert(thread);
    assert(threads_.count(thread) == 1);

    add_thread_internal(thread);
}


void UnixTarget::unmanage_thread(const RefPtr<Thread>& thread)
{
    if (remove_thread_internal(thread))
    {
        assert(threads_.find(thread) != threads_.end());
        threads_.erase(thread);
    }
    else
    {
        assert(threads_.find(thread) == threads_.end());
    }
}


void UnixTarget::decrement_iter_count()
{
    assert(iterCount_);

    if (--iterCount_ == 0)
    {
        // once there are no more outstanding iterators it is
        // safe to modify the internal maps / sets of threads

        // deal with pending deletions
        vector<RefPtr<Thread> >::const_iterator i(threadsToDelete_.begin());
        for (; i != threadsToDelete_.end(); ++i)
        {
            unmanage_thread(*i);
        }
        threadsToDelete_.clear();

        // deal with pending additions
        for (i = threadsToAdd_.begin(); i != threadsToAdd_.end(); ++i)
        {
            manage_thread(*i);
        }
        threadsToAdd_.clear();
    }
}


void UnixTarget::add_thread(const RefPtr<Thread>& thread)
{
    if (iterCount_ == 0)
    {
        manage_thread(thread);
    }
    else
    {
        // defer adding the thread
        threadsToAdd_.push_back(thread);
    }
}


void UnixTarget::remove_thread(const RefPtr<Thread>& thread)
{
    if (iterCount_)
    {
        threadsToDelete_.push_back(thread);
        dbgout(1) << __func__ << ": " << thread->lwpid() << " pending" << endl;
    }
    else
    {
        unmanage_thread(thread);
    }
}


bool
UnixTarget::is_deletion_pending(const RefPtr<Thread>& thread) const
{
    std::vector<RefPtr<Thread> >::const_iterator j =
        threadsToDelete_.begin();

    for (; j != threadsToDelete_.end(); ++j)
    {
        if (thread == *j)
        {
            //dbgout(0) << __func__ << ": " << thread->lwpid() << endl;
            return true;
        }
    }
    return false;
}


bool UnixTarget::is_deletion_pending(pid_t lwpid) const
{
    std::vector<RefPtr<Thread> >::const_iterator j =
        threadsToDelete_.begin();

    for (; j != threadsToDelete_.end(); ++j)
    {
        if (lwpid == (*j)->lwpid())
        {
            dbgout(0) << __func__ << ": " << lwpid << endl;
            return true;
        }
    }
    return false;
}


void UnixTarget::reset_type_system()
{
    types_.reset(new NativeTypeSystem(wordSize_));
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
