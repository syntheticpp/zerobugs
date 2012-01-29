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

#include <algorithm>
#include <iostream>
#ifdef HAVE_SYS_PARAM_H
 #include <sys/param.h>
#endif
#ifdef HAVE_SYS_USER_H
 #include <sys/user.h>
#endif
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "dharma/symbol_util.h"
#include "generic/lock.h"
#include "generic/state_saver.h"
#include "debugger_engine.h"
#include "watchpoint.h"
#include "breakpoint_mgr.h"

using namespace std;

#define THREAD_SAFE Lock<Mutex> lock(mutex_)



////////////////////////////////////////////////////////////////
bool BreakPointManagerBase::enable_software_breakpoint(
    Thread& thread,
    addr_t  addr,
    bool    enable)
{
    PhysicalBreakPoint& brkpnt = map_[addr];

    if (brkpnt.origCode == 0)
    {
        if (brkpnt.enabled)
        {
            ostringstream err;
            err << "breakpoint already enabled at ";
            if (Symbol* sym = thread.symbols()->lookup_symbol(addr))
            {
                err << sym->demangled_name()->c_str();
            }
            else
            {
                err << hex << addr;
            }
            throw logic_error(err.str());
        }
        try
        {
            thread.read_code(addr, &brkpnt.origCode, 1);
        }
        catch (const exception& e)
        {
            clog << __func__ << ": " << e.what() << endl;
        }
    }
    // failed using the thread for reading?
    // fallback to using the process object
    if (brkpnt.origCode == 0)
    {
        process_->read_code(addr, &brkpnt.origCode, 1);
    }

    word_t code = brkpnt.origCode; // code to be poked into debuggee
    if (enable)
    {
        code = CHKPTR(thread.target())->get_breakpoint_opcode(code);
    }
    if (code)
    {
        if (enable != brkpnt.enabled)
        {
            try
            {
                thread.write_code(addr, &code, 1);
                brkpnt.enabled = enable;
            }
            catch (const exception& e)
            {
                clog << __func__ << " " << e.what() << endl;
            }
        }
        // fallback to writing debugged program's code segment
        // using the process object
        if (enable != brkpnt.enabled)
        {
            try
            {
                process_->write_code(addr, &code, 1);
                brkpnt.enabled = enable;
            }
            catch (...)
            {
                return false;
            }
        }
    }
    // todo: remove from map when no longer referenced?
    // even if we end up inserting thousands of breakpoints,
    // is it worth the extra coding effort?
    return true;
}


////////////////////////////////////////////////////////////////
BreakPointManagerImpl::BreakPointManagerImpl
(
    RefPtr<Process> process,
    int             verbose,       // verbosity level
    bool            useHardware,   // use hardware support?
    Callback        onInsert,
    Callback        onRemove
)
  : BreakPointManagerBase(process)
  , verbose_(verbose)
  , useHardware_(useHardware)
  , onInsert_(onInsert)
  , onRemove_(onRemove)
{
}


////////////////////////////////////////////////////////////////
BreakPointManagerImpl::~BreakPointManagerImpl() throw()
{
    assert(ref_count() == 0);
}


////////////////////////////////////////////////////////////////
template<typename T>
static BreakPointBase& breakpoint_base(T bpnt)
{
    if (BreakPointBase* base =
        interface_cast<BreakPointBase*>(const_cast<BreakPoint*>(bpnt)))
    {
        return *base;
    }
    throw logic_error("invalid BreakPoint implementation");
}


////////////////////////////////////////////////////////////////
void BreakPointManagerImpl::clone_breakpoint(volatile BreakPoint* bpnt)
{
    if (bpnt && bpnt->thread() && process())
    {
        BreakPointBase& base = breakpoint_base(bpnt);

        assert(bpnt->thread());
        assert(bpnt->thread()->lwpid() != pid());

        RefPtr<BreakPointBase> bpnt2;

        if (Thread* thread2 = process()->get_thread(DEFAULT_THREAD))
        {
            bpnt2 = base.clone(this, thread2);
        }
        if (bpnt2)
        {
            BreakPointMap* bpntMap = &globalBreakPoints_;

            if (bpnt->type() != BreakPoint::GLOBAL)
            {
                bpntMap = &perThreadBreakPoints_[pid()];
            }
            bpntMap->insert(make_pair(bpnt2->addr(), bpnt2));
        }
    }
}


////////////////////////////////////////////////////////////////
void BreakPointManagerImpl::notify(volatile BreakPoint* bpnt)
{
    clone_breakpoint(bpnt);
}


////////////////////////////////////////////////////////////////
bool BreakPointManagerImpl::on_thread_created(Thread& thread)
{
    THREAD_SAFE;
    bool handled = false;

    if (thread.is_live() && thread_in_process(thread, pid()))
    {
        if (thread.is_execed())
        {
            assert(perThreadBreakPoints_.empty());
            assert(globalBreakPoints_.empty());
            assert(deferred_.empty());
            assert(watchPoints_.empty());
        }

        perThreadBreakPoints_[thread.lwpid()];
        handled = true;
    }
    return handled;
}


typedef BreakPointManagerImpl::BreakPointMap BreakPointMap;


////////////////////////////////////////////////////////////////
static void keep_enabled(BreakPointMap& breakPointMap)
{
    BreakPointMap::iterator bi = breakPointMap.begin();
    for (; bi != breakPointMap.end(); ++bi)
    {
        // intentionaly bump up the "enable" count so
        // that the breakpoint does not attempt to disable
        // itself on a non-existing thread

        if (bi->second->is_enabled())
        {
            bi->second->enable();
        }
    }
}


////////////////////////////////////////////////////////////////
void BreakPointManagerImpl::on_thread_unmanage(Thread& thread)
{
    THREAD_SAFE;
    assert(thread_in_process(thread, pid()));

    if (thread.is_live())
    {
        const pid_t lwpid = thread.lwpid();

        dbgout(0) << __func__ << ": pid=" << lwpid << endl;

        PerThreadBreakPoints::iterator iter =
            perThreadBreakPoints_.find(lwpid);
        if (iter != perThreadBreakPoints_.end())
        {
            keep_enabled(iter->second);
            perThreadBreakPoints_.erase(iter);
        }

        if (Process* proc = thread.process())
        {
            // is this the last thread in its process?

            if ((proc->pid() == pid()) && (proc->enum_threads() <= 1)
              && proc->get_thread(lwpid, thread.thread_id()))
            {
                keep_enabled(globalBreakPoints_);
                globalBreakPoints_.clear();
                dbgout(0) << __func__ << ": erased global breakpoints." << endl;
            }
        }

        // remove all watchpoints for this thread
        for (auto i = watchPoints_.begin(); i != watchPoints_.end(); )
        {
            if ((*i)->thread() == &thread)
            {
                i = watchPoints_.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }
}


////////////////////////////////////////////////////////////////
RefPtr<BreakPoint> BreakPointManagerImpl::get_breakpoint(
    const Process& p,
    addr_t a,
    const Thread* owner) const
{
    RefPtr<BreakPoint> result = get_breakpoint_impl(p, a);
    if (!result)
    {
        // Check if an emulated breakpoint exists at the address
        // for a different thread. Because emulated breakpoints are
        // software breakpoints and thus physically shared, it is
        // possible for another thread than the owner to get a SIGTRAP.
        //
        // Before the thread is resumed, the program counter needs to be
        // decremented; the logic inside debugger_engine.cpp that does this
        // needs a breakpoint object.

        PerThreadBreakPoints::const_iterator i = perThreadBreakPoints_.begin();
        for (; i != perThreadBreakPoints_.end(); ++i)
        {
            const BreakPointMap& m = i->second;
            BreakPointMap::const_iterator j = m.find(a);
            if (j != m.end())
            {
                if (!j->second->is_enabled())
                {
                    continue;
                }
                if (j->second->type() == BreakPoint::EMULATED)
                {
                    if (owner && j->second->thread() != owner)
                    {
                        continue;
                    }
                    result = j->second;
                    break;
                }
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
const BreakPointMap*
BreakPointManagerImpl::get_breakpoint_map(const Thread& thread) const
{
    const BreakPointMap* mptr = NULL;

    if (RefPtr<Process> process = thread.process())
    {
        if (process->pid() == pid())
        {
            PerThreadBreakPoints::const_iterator k =
                perThreadBreakPoints_.find(thread.lwpid());

            if (k != perThreadBreakPoints_.end())
            {
                mptr = &k->second;
            }
        }
    }
    return mptr;
}


////////////////////////////////////////////////////////////////
RefPtr<WatchPoint> BreakPointManagerImpl::get_watchpoint(
    Thread*     thread,
    addr_t      addr,
    WatchType   type) const
{
    THREAD_SAFE;
    RefPtr<WatchPoint> result;

    vector<RefPtr<WatchPoint> >::const_iterator
        i = watchPoints_.begin(),
        end = watchPoints_.end();

    for (; i != end && !result; ++i)
    {
        RefPtr<WatchPoint> wp = *i;

        if (wp->addr() == addr
         && wp->watch_type() == type
         && (wp->is_global() || wp->thread() == thread))
        {
            if (wp->in_scope(thread))
            {
                result = wp;
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
//
// activate a deferred breakpoint
//
void BreakPointManagerImpl::activate(
    RefPtr<BreakPoint> bpnt,
    const SymbolTable& symTab)
{
    assert(bpnt);
    assert(bpnt->is_deferred());

    Runnable* runnable = interface_cast<Runnable*>(bpnt->thread());

    BreakPointBase& old = breakpoint_base(bpnt.get());
    BreakPointBase::ActionList actions = old.actions();

    BreakPointBase::ActionList::const_iterator i = actions.begin();
    for (; i != actions.end(); ++i)
    {
        const addr_t addr = bpnt->addr() + symTab.adjustment();
        set_breakpoint(runnable, bpnt->type(), addr, i->get());
    }
    if (symTab.is_loaded())
    {
        old.remove_actions();
        erase(bpnt);
    }
}


////////////////////////////////////////////////////////////////
void BreakPointManagerImpl::erase(RefPtr<BreakPoint> bpnt)
{
    THREAD_SAFE;
    assert(bpnt);
    assert(bpnt->enum_actions() == 0);
    assert(bpnt->thread());

    if (!thread_in_process(*bpnt->thread(), pid()))
    {
        return;
    }
    if (bpnt->is_deferred())
    {
        BreakPointList::iterator i = deferred_.begin();
        for (; i != deferred_.end(); ++i)
        {
            if (bpnt.get() == i->get())
            {
                deferred_.erase(i);
                call_callback(onRemove_, bpnt.get());
                break;
            }
        }
        dbgout(0) << __func__ << ": deferred" << endl;
        return;
    }

    // if watchpoint, remove it from watchPoints_ vector
    if (WatchPoint* wp = interface_cast<WatchPoint*>(bpnt.get()))
    {
        vector<RefPtr<WatchPoint> >::iterator i(watchPoints_.begin());
        for (; i != watchPoints_.end(); ++i)
        {
            if (i->get() == wp)
            {
                watchPoints_.erase(i);

                // is it needed to call notification callback here?
                //call_callback(onRemove_, bpnt.get());

                break;
            }
        }
        return;
    }
    BreakPointMap* mptr = 0;

    if (bpnt->type() == BreakPoint::SOFTWARE)
    {
        mptr = &globalBreakPoints_;
    }
    else
    {
        mptr = &perThreadBreakPoints_[bpnt->thread()->lwpid()];
    }
    assert(mptr);
    BreakPointMap::iterator i = mptr->find(bpnt->addr());

    if (i != mptr->end())
    {
        call_callback(onRemove_, bpnt.get());
        mptr->erase(i);
    }
    else
    {
        // report the error since, most likely, it's a
        // programming mistake if we get to this point
        StateSaver<ios, ios::fmtflags>__(cerr);

        cerr << __func__ << ": " << bpnt->type();
        cerr << " breakpoint not found at ";
        cerr << hex << bpnt->addr() << dec << endl;
        cerr << "This group PID=" << pid() << ", breakpoint PID=";
        cerr << bpnt->thread()->lwpid() << endl;
    }
}


////////////////////////////////////////////////////////////////
static void check_set_breakpoint_contract_in(
    pid_t               pid,
    Thread*             thread,
    BreakPoint::Type    type)
{
    if (!CHKPTR(thread)->is_live())
    {
        throw runtime_error("operation requires a live thread");
    }

    if (type == BreakPoint::SOFTWARE)
    {
        // are we still attached to the main thread?
        if (!thread_is_attached(*thread))
        {
            throw logic_error("thread not attached");
        }
    }
    if (!thread_in_process(*thread, pid))
    {
        assert(false);
        throw logic_error("thread belongs to another process");
    }
}


////////////////////////////////////////////////////////////////
template<typename T>
static bool breakpoint_matches(T bpnt,
                        Thread* thread,
                        addr_t addr,
                        BreakPoint::Type type)
{
    return (thread == 0 || bpnt->thread() == thread)
          && (addr == 0 || bpnt->addr() == addr)
          && (type == BreakPoint::ANY || type == bpnt->type());
}


////////////////////////////////////////////////////////////////
//
// If a breakpoint already exists at the given address,
// just add a new action to it; otherwise, create a new
// breakpoint object.
//
BreakPointBase* BreakPointManagerImpl::set_breakpoint(
    Runnable*           runnable,
    BreakPoint::Type    type,
    addr_t              addr,
    BreakPoint::Action* action,
    bool                deferred,
    Symbol*             sym)
{
    THREAD_SAFE;
    assert((sym == NULL) || (sym->addr() == addr));

    Thread* thread = get_thread(runnable);
    if (thread)
    {
        check_set_breakpoint_contract_in(pid(), thread, type);
    }
    bool isNewBreakPoint = false;

    BreakPointMap* mptr = NULL;

    // look for an existing breakpoint at the given address
    RefPtr<BreakPointBase> bpnt(find_breakpoint(thread, type, addr, mptr));
    assert(mptr); // post-condition

    if (!bpnt)
    {
        bpnt = find_deferred(thread, type, addr);
    }
    if (!bpnt)
    {
        if (deferred)
        {
            bpnt = new DeferredBreakPoint(this, thread, type, addr);
            deferred_.push_back(bpnt);
        }
        else
        {
            bpnt = create_breakpoint(thread, type, addr);

            assert(bpnt);
            assert(mptr->find(addr) == mptr->end());
            (*mptr)[addr] = bpnt;
        }
        bpnt->set_symbol(sym);
        isNewBreakPoint = true;
    }
    if (action)
    {
        assert(bpnt);
        bpnt->add_action(action);
    }
    if (isNewBreakPoint)
    {
        // Call the callback after the breakpoint and the
        // associated action have been inserted, so that
        // if the client code calls enumerate_breakpoints
        // or enumerate_actions from within the on_insert
        // callback, it will see the changes we just made
        // (the new action added).

        call_callback(onInsert_, bpnt.get());
    }
    return bpnt.get();
}



////////////////////////////////////////////////////////////////
RefPtr<BreakPointBase> BreakPointManagerImpl::create_breakpoint(
    Thread*             thread,
    BreakPoint::Type    type,
    addr_t              addr
)
{
    RefPtr<BreakPointBase> bpnt;
    uint32_t reg = 0;

    // Try enabling a local (per-thread) hardware breakpoint.
    //
    // There is a bit of a chicken and egg problem here. Before
    // I create a HardwareBreakpoint, I need to know what hardware
    // debug register to use, and thus the accessing of the debug
    // regs cannot be completely encapsulated inside the
    // HardwareBreakpoint class -- the breakpoint manager needs
    // to call into the lower level DebugRegs object directly

    if ((type == BreakPoint::HARDWARE)
        && use_hardware()
        && set_hardware_breakpoint(*thread, addr, &reg))
    {
        dbgout(0) <<__func__<< ": " << pid() << '/'<< thread->lwpid()
                  << " using hardware debug reg " << reg
                  << " at " << (void*) addr << endl;

        bpnt.reset(new HardwareBreakPoint(thread, addr, reg));
    }
    else
    {
        if (type == BreakPoint::SOFTWARE)
        {
            bpnt.reset(new SoftwareBreakPoint(this, thread, addr));
        }
        else
        {
            // We get here either because of falling back from not
            // being able to set a hardware breakpoint, and we need
            // to emulate the behavior: only trigger the breakpoint
            // actions on the thread that matches pid; or: the
            // caller has explicitly requested a
            // PER_THREAD breakpoint.
            bpnt.reset(new EmulatedBreakPoint(this, thread, addr));
        }
        // hardware breakpoints are created in an enabled state,
        // but in the case of  emulated and software breakpoints
        // we need to make an explicit call into the enable method
        bpnt->enable();
    }
    return bpnt;
}


////////////////////////////////////////////////////////////////
BreakPoint* BreakPointManagerImpl::set_watchpoint(
    Runnable*           runnable,
    WatchType           type,
    bool                global,
    addr_t              addr,
    BreakPoint::Action* action)
{
    Thread* thread = runnable ? runnable->thread() : NULL;

    THREAD_SAFE;
    RefPtr<WatchPoint> watchPoint = get_watchpoint(thread, addr, type);

    if (!watchPoint)
    {
        DebugRegs::Condition cond = DebugRegs::BREAK_ON_DATA_WRITE;

        if (type == WATCH_READ_WRITE)
        {
            dbgout(0) << __func__ << ": on_data_read_write" << endl;

            cond = DebugRegs::BREAK_ON_DATA_READ_WRITE;
        }
        uint32_t reg = 0;

        if (set_hardware_breakpoint(*thread, addr, &reg, global, cond))
        {
            dbgout(0) << __func__ << ": " << pid() << '/'<< thread->lwpid()
                      << " using hardware debug reg " << reg << endl;
            dbgout(0) << __func__ << ": type=" << type << ", cond=" << cond << endl;

            watchPoint = new WatchPoint(thread, type, addr, reg, global, cond);

            if (thread && addr < thread->stack_start())
            {
                if (RefPtr<Symbol> fun = thread_current_function(thread))
                {
                    const addr_t scope = fun->addr() - fun->offset();
                    watchPoint->set_scope(scope);
                   /*
                    if (verbose())
                    {
                        clog << __func__ << ": on stack: ";
                        print_symbol(clog, scope, fun);
                        clog << endl;
                    } */
                }
            }
            watchPoints_.push_back(watchPoint);
        }
    }
    if (action && watchPoint)
    {
        watchPoint->add_action(action);
    }
    assert(!watchPoint || watchPoint->ref_count() >= 2);
    return watchPoint.get();
}


typedef EnumCallback<volatile BreakPoint*> BreakPointCallback;

////////////////////////////////////////////////////////////////
size_t BreakPointManagerImpl::enum_deferred(
    Thread*             thread,
    addr_t              addr,
    BreakPoint::Type    type,
    BreakPointCallback* callback
    ) const
{
    size_t count = 0;

    for (auto i = deferred_.begin(); i != deferred_.end(); ++i)
    {
        if (breakpoint_matches(*i, thread, addr, type))
        {
            if (callback) callback->notify(i->get());
            ++count;
        }
    }
    return count;
}


////////////////////////////////////////////////////////////////
RefPtr<BreakPointBase> BreakPointManagerImpl::find_deferred(

    Thread*             thread,
    BreakPoint::Type    type,
    addr_t              addr
    )const
{
    BreakPointList::const_iterator i = deferred_.begin();

    for (; i != deferred_.end(); ++i)
    {
        if (breakpoint_matches(*i, thread, addr, type))
        {
            return *i;
        }
    }
    return RefPtr<BreakPointBase>();
}



////////////////////////////////////////////////////////////////
size_t BreakPointManagerImpl::enum_breakpoints(

    BreakPointMap           breakpoints,
    addr_t                  addr,
    BreakPoint::Type        type,
    BreakPointCallback*     callback
)
{
    size_t result = 0;

    for (auto i = breakpoints.begin(); i != breakpoints.end(); ++i)
    {
        BreakPoint* bpnt = i->second.get();

        if ((addr == 0 || addr == bpnt->addr())
         && (type == BreakPoint::ANY || type == bpnt->type()))
        {
            ++result;

            if (callback)
            {
                callback->notify(bpnt);
            }
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerImpl::enum_breakpoints(

    BreakPointCallback* callback,
    Thread*             thread,
    addr_t              addr,
    BreakPoint::Type    type
    ) const
{
    size_t resultCount = 0;

    THREAD_SAFE;
    const BreakPointMap* mptr = &globalBreakPoints_;

    if (thread)
    {
        auto k = perThreadBreakPoints_.find(thread->lwpid());

        assert(k != perThreadBreakPoints_.end());
        mptr = &k->second;
    }
    assert(mptr);

    resultCount = enum_breakpoints(*mptr, addr, type, callback);

    if (!thread)
    {
        auto k = perThreadBreakPoints_.begin();

        for (; k != perThreadBreakPoints_.end(); ++k)
        {
            resultCount += enum_breakpoints(k->second, addr, type, callback);
        }
    }

    if ((resultCount == 0) || (thread == 0) || (addr == 0))
    {
        resultCount += enum_deferred(thread, addr, type, callback);
    }
    return resultCount;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerImpl::enum_watchpoints(
    BreakPointCallback* callback) const
{
    THREAD_SAFE;
    size_t result = 0;

    // Use a temp copy of the container, so that the iterators
    // are valid for the duration of the iteration, even if the
    // client code erases watchpoints from within the callback.
    vector<RefPtr<WatchPoint> > watchPoints(watchPoints_);

    vector<RefPtr<WatchPoint> >::const_iterator j(watchPoints.begin());
    for (; j != watchPoints.end(); ++j)
    {
        ++result;

        if (callback)
        {
            callback->notify(j->get());
        }
    }

    return result;
}


////////////////////////////////////////////////////////////////
void BreakPointManagerImpl::call_callback
(
    Callback callback,
    volatile BreakPoint* bpnt
)
{
    assert(bpnt);

    if (!callback)
    {
        return;
    }
    assert(bpnt->thread());

    if (Debugger* dbg = bpnt->thread()->debugger())
    {
        DebuggerEngine& eng = interface_cast<DebuggerEngine&>(*dbg);
        (eng.*callback)(bpnt);
    }
}


////////////////////////////////////////////////////////////////
unsigned int BreakPointManagerImpl::hardware_breakpoint_count(pid_t tid) const
{
    THREAD_SAFE;
    size_t count = 0;

    PerThreadBreakPoints::const_iterator k(perThreadBreakPoints_.find(tid));

    if (k != perThreadBreakPoints_.end())
    {
        const BreakPointMap& m = k->second;

        BreakPointMap::const_iterator i = m.begin();
        for (; i != m.end(); ++i)
        {
            if (interface_cast<HardwareBreakPoint*>(i->second.get()))
            {
                ++count;
            }
        }
    }

    vector<RefPtr<WatchPoint> >::const_iterator j(watchPoints_.begin());

    for (; j != watchPoints_.end(); ++j)
    {
        if ((*j)->thread()->lwpid() == tid)
        {
            ++count;
        }
    }
    return count;
}


////////////////////////////////////////////////////////////////
bool BreakPointManagerImpl::set_hardware_breakpoint(
    Thread& thread,
    addr_t addr,
    uint32_t* reg,
    bool global,
    HardwareBreakPoint::Condition cond,
    HardwareBreakPoint::Length len
)
{
    THREAD_SAFE;
    bool result = false;

    unsigned int count = hardware_breakpoint_count(thread.lwpid());
    if (DebugRegs* regs = thread.debug_regs())
    {
        if (count < regs->max_size(cond))
        {
            result = regs->set_breakpoint(addr, reg, global, cond, len);
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerImpl::remove_actions(
    BreakPointMap& m,
    BreakPointMap::iterator& i,
    const char* name
)
{
    size_t removedCount = CHKPTR(i->second)->remove_actions(name);

    if (i->second->enum_actions() == 0)
    {
        RefPtr<BreakPoint> brkPnt = i->second;
        brkPnt->disable();

        // announce removal to observers
        call_callback(onRemove_, brkPnt.get());

        m.erase(i++);
    }
    else
    {
        ++i;
    }
    return removedCount;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerImpl::remove_actions (
    BreakPointMap& m,
    addr_t addr,
    const char* name)
{
    size_t removedCount = 0;
    if (addr)
    {
        BreakPointMap::iterator i = m.find(addr);
        if (i != m.end())
        {
            removedCount += remove_actions(m, i, name);
        }
    }
    else
    {
        BreakPointMap::iterator i = m.begin();
        while (i != m.end())
        {
            removedCount += remove_actions(m, i, name);
        }

    }
    return removedCount;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerImpl::remove_breakpoint_actions (
    pid_t       pid,
    pid_t       lwpid, // todo FreeBSD: use thread_id instead
    addr_t      addr,
    const char* name)
{
    size_t removedCount = 0;
    assert((pid == 0) || (pid == this->pid()));

    PerThreadBreakPoints::iterator i = perThreadBreakPoints_.begin();
    if (lwpid)
    {
        i = perThreadBreakPoints_.find(lwpid);
        if (i != perThreadBreakPoints_.end())
        {
            removedCount = remove_actions(i->second, addr, name);
        }
    }
    else
    {
        if (!pid)
        {
            for (; i != perThreadBreakPoints_.end(); ++i)
            {
                removedCount += remove_actions(i->second, addr, name);
            }
        }
        removedCount += remove_actions(globalBreakPoints_, addr, name);
    }

    // let's try the deferred breakpoints
    if (removedCount == 0)
    {
        BreakPointList::iterator i = deferred_.begin();

        while (i != deferred_.end())
        {
            if ((addr == 0 || addr == (*i)->addr())
              &&(!lwpid || !(*i)->thread() || (*i)->thread()->lwpid() == lwpid)
               )
            {
                if ((*i)->remove_actions(name) && (*i)->enum_actions() == 0)
                {
                    call_callback(onRemove_, i->get());

                    i = deferred_.erase(i);
                    continue;
                }
            }
            ++i;
        }
    }
    return removedCount;
}


////////////////////////////////////////////////////////////////
RefPtr<BreakPointBase> BreakPointManagerImpl::find_breakpoint(
    Thread*             thread,
    BreakPoint::Type    type,
    addr_t              addr,
    BreakPointMap*&     mptr)
{
    assert(type != BreakPoint::ANY);
    RefPtr<BreakPointBase> bpnt;

    // software breakpoints are global (i.e. apply to all threads)
    mptr = &globalBreakPoints_;

    // hardware breakpoints work on a per-thread basis
    if (type != BreakPoint::GLOBAL)
    {
        mptr = &perThreadBreakPoints_[thread->lwpid()];
    }
    BreakPointMap::const_iterator i = mptr->find(addr);
    if (i != mptr->end())
    {
        bpnt = i->second;
    }
    return bpnt;
}



////////////////////////////////////////////////////////////////
Thread* BreakPointManagerImpl::get_thread(Runnable* runnable) const
{
    Thread* thread = NULL;

    if (runnable)
    {
        thread = runnable->thread();
    }
    else if (Process* proc = process())
    {
        assert(proc->origin() != ORIGIN_CORE);
        thread = proc->get_thread(DEFAULT_THREAD);
    }
    return thread;
}


////////////////////////////////////////////////////////////////
size_t BreakPointManagerImpl::reset( pid_t )
{
    return 0;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
