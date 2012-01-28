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

#include <iostream>
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "dbgout.h"
#include "debugger_engine.h"
#include "watchpoint.h"
#include "breakpoint_enabler.h"
#include "breakpoint_mgr.h"

using namespace std;
using namespace eventlog;


BreakPointManagerGroup::BreakPointManagerGroup
(
    RefPtr<Process> process,
    int  verbose,       // verbosity level
    bool useHardware,   // use hardware support?
    Callback onInsert,
    Callback onRemove
)
  : BreakPointManagerBase(process)
  , verbose_(verbose)
  , useHardware_(useHardware)
  , onInsert_(onInsert)
  , onRemove_(onRemove)
{
    RefPtr<BreakPointManagerImpl> mgr =
        new BreakPointManagerImpl(  process,
                                    verbose_,
                                    useHardware_,
                                    onInsert_,
                                    onRemove_);
    group_.push_back(mgr);
}


////////////////////////////////////////////////////////////////
BreakPointManagerGroup::~BreakPointManagerGroup() throw()
{
}



////////////////////////////////////////////////////////////////
void
BreakPointManagerGroup::replicate_breakpoints (
    Thread& forkedThread,
    BreakPointManagerImpl& mgr
)
{
    assert(forkedThread.is_forked());

    // forked thread must have a parent, group should not be empty
    assert(!group_.empty());

    const pid_t parentID = forkedThread.ppid();

    // Do a linear search for pids that match the parent pid;
    // (linear search is not that bad, do not expect too many
    // elements in the group anyway).

    for (Group::const_iterator i = group_.begin(); i != group_.end(); ++i)
    {
        if ((*i)->pid() == parentID)
        {
            // Under  Linux,  fork()  is implemented using
            // copy-on-write pages; this means that all software
            // breakpoints that are currently poked into the
            // parent will also be present in the forked child

            mgr.set_physical_map((*i)->physical_map());

            // clone breakpoints
            (*i)->enum_breakpoints(&mgr);

        #if 0
            clog << "===== PHYSICAL BREAKPOINT MAP =====\n";
            const PhysicalMap& pm = mgr.physical_map();
            for (PhysicalMap::const_iterator i = pm.begin();
                 i != pm.end();
                 ++i)
            {
                clog << "\t" << (void*)i->first << ": ";
                clog << i->second.enabled << " ";
                clog << (void*)i->second.origCode << endl;
            }
        #endif
            return;
        }
    }
    ostringstream msg;
    msg << "Failed to replicate breakpoints in forked thread(";
    msg << forkedThread.lwpid() << ", " << parentID << ")";

    throw runtime_error(msg.str());
}


////////////////////////////////////////////////////////////////
bool BreakPointManagerGroup::on_thread_created(Thread& thread)
{
    assert(!thread.is_execed());

    bool handled = false;

    if (thread.is_live())
    {
        if (thread.is_forked())
        {
            Process* process = CHKPTR(thread.process());

            // create a new breakpoint manager for the forked thread

            RefPtr<BreakPointManagerImpl> mgr =
                new BreakPointManagerImpl(  process,
                                            verbose_,
                                            useHardware_,
                                            onInsert_,
                                            onRemove_);

            handled = mgr->on_thread_created(thread);
            replicate_breakpoints(thread, *mgr);

            group_.push_back(mgr);
        }
        else
        {
            // Notify all managers in group that a new thread has
            // been created; the breakpoint manager for the process
            // where the new thread belongs picks up the notification,
            // all others ignore it.

            for (auto i = group_.begin(); i != group_.end() && !handled; ++i)
            {
                handled = (*i)->on_thread_created(thread);
            }
        }
    }
    return handled;
}


////////////////////////////////////////////////////////////////
void BreakPointManagerGroup::on_thread_unmanage(Thread& thread)
{
    if (thread.is_live())
    {
        Group::iterator i = group_.begin();
        for (; i != group_.end(); )
        {
            Process* proc = thread.process();
            if (!proc || proc->pid() != (*i)->pid())
            {
                ++i;
            }
            else
            {
                if (thread_finished(thread) || proc->enum_threads())
                {
                    (*i)->on_thread_unmanage(thread);
                    ++i;
                }
                else
                {
                    // last thread left in process
                    BreakPointEnabler disabler(false, BreakPoint::ANY);
                    (*i)->enum_breakpoints(&disabler);

                    i = group_.erase(i);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////
void BreakPointManagerGroup::on_exec(Thread& thread)
{
    Process* process = CHKPTR(thread.process());

    // create a brand new breakpoint manager
    RefPtr<BreakPointManagerImpl> mgr =
        new BreakPointManagerImpl(  process,
                                    verbose_,
                                    useHardware_,
                                    onInsert_,
                                    onRemove_);
    mgr->on_thread_created(thread);

    Group::iterator i = group_.begin();
    while (i != group_.end())
    {
        if ((*i)->pid() == mgr->pid())
        {
        #if DEBUG
            clog << "Replacing breakpoint manager for process: " 
                 << process->pid() << endl;
        #endif

            *i = mgr; // replace the old manager
            return;
        }
        ++i;
    }

    // just in case (although we should never get here):
    group_.push_back(mgr);
}



////////////////////////////////////////////////////////////////
RefPtr<WatchPoint> BreakPointManagerGroup::get_watchpoint(
    Thread*     thread,
    addr_t      addr,
    WatchType   type) const
{
    RefPtr<WatchPoint> result;

    Group::const_iterator i = group_.begin();
    for (; i != group_.end() && !result; ++i)
    {
        result = (*i)->get_watchpoint(thread, addr, type);
    }
    return result;
}


////////////////////////////////////////////////////////////////
void BreakPointManagerGroup::erase(RefPtr<BreakPoint> bpnt)
{
    assert(bpnt);
    assert(bpnt->enum_actions() == 0);

    Group::iterator i = group_.begin();
    for (; i != group_.end(); ++i)
    {
        (*i)->erase(bpnt);
    }
}


////////////////////////////////////////////////////////////////
/// activate deferred breakpoint
void BreakPointManagerGroup::activate(

    RefPtr<BreakPoint> bpnt,
    const SymbolTable& symTab)
{
    assert(bpnt);

    if (!bpnt->is_deferred())
    {
        throw invalid_argument(__func__);
    }

    Process* process = bpnt->thread() ? bpnt->thread()->process() : NULL;

    for (Group::iterator i = group_.begin(); i != group_.end(); ++i)
    {
        if (process && process->pid() != (*i)->pid())
        {
            continue;
        }

        (*i)->activate(bpnt, symTab);
    }
}


////////////////////////////////////////////////////////////////
BreakPointBase* BreakPointManagerGroup::set_breakpoint(
    Runnable*           runnable,
    BreakPoint::Type    type,
    addr_t              addr,
    BreakPoint::Action* action,
    bool                deferred,
    Symbol*             sym)
{
    BreakPointBase* result = 0;

    if (sym)
    {
        addr = sym->addr();
    }

    for (Group::iterator i(group_.begin()); i != group_.end(); ++i)
    {
        // This prevents breakpoints to be inserted in fork()ed and
        // exec-ed processes

        if (runnable && runnable->process() != (*i)->process())
        {
            dbgout(1) << __func__ << ": not in process " << (*i)->pid() << endl;
            continue;
        }

        result = (*i)->set_breakpoint(runnable,
                                      type,
                                      addr,
                                      action,
                                      deferred,
                                      sym);

        dbgout(0) << __func__ << ": addr=" << (void*)addr;
        dbgout(0) << " [" << (runnable ? runnable->pid() : 0) << "]";
        dbgout(0) << " type=" << type << ": " << result << endl;

        if (result)
        {
            // add software breakpoints to all groups
            if (type != BreakPoint::SOFTWARE)
            {
                break;
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
BreakPoint* BreakPointManagerGroup::set_watchpoint(
    Runnable*           runnable,
    WatchType           type,
    bool                global,
    addr_t              addr,
    BreakPoint::Action* action)
{
    BreakPoint* result = 0;

    Group::iterator i = group_.begin();
    for (; i != group_.end() && !result; ++i)
    {
        result = (*i)->set_watchpoint(runnable,
                                      type,
                                      global,
                                      addr,
                                      action);
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerGroup::enum_breakpoints(

    BreakPointCallback* callback,
    Thread*             thread,
    addr_t              addr,
    BreakPoint::Type    type
    ) const
{
    size_t count = 0;

    for (Group::const_iterator i = group_.begin(); i != group_.end(); ++i)
    {
        count += (*i)->enum_breakpoints(callback, thread, addr, type);
    }
    return count;
}



////////////////////////////////////////////////////////////////
size_t BreakPointManagerGroup::enum_watchpoints(BreakPointCallback* cb) const
{
    size_t count = 0;

    for (Group::const_iterator i = group_.begin(); i != group_.end(); ++i)
    {
        count += (*i)->enum_watchpoints(cb);
    }
    return count;
}


////////////////////////////////////////////////////////////////
BreakPointManager* BreakPointManagerGroup::get_manager(pid_t pid) const
{
    for (Group::const_iterator i = group_.begin(); i != group_.end(); ++i)
    {
        if ((*i)->pid() == pid)
        {
            return (*i).get();
        }
    }
    return NULL;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerGroup::reset(pid_t pid)
{
    for (Group::iterator i = group_.begin(); i != group_.end(); ++i)
    {
        if ((*i)->pid() == pid)
        {
            group_.erase(i);
            clog << "Reset breakpoints manager for pid: " << pid << endl;

            return 1;
        }
    }
    clog << "breakpoints manager for pid: " << pid <<  " not found" << endl;
    return 0;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerGroup::remove_breakpoint_actions(
    pid_t       pid,
    pid_t       lwpid,
    addr_t      addr,
    const char* name)
{
    size_t removedCount = 0;

    for (Group::const_iterator i = group_.begin(); i != group_.end(); ++i)
    {
        // pid == 0 matches all
        if (pid && (*i)->pid() != pid)
        {
            continue;
        }
        removedCount += (*i)->remove_breakpoint_actions(pid, lwpid, addr, name);
    }
    return removedCount;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
