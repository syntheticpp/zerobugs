#ifndef BREAKPOINT_MGR_H__9A0FCE8C_CBCC_4709_A0EC_694971AF6575
#define BREAKPOINT_MGR_H__9A0FCE8C_CBCC_4709_A0EC_694971AF6575
//
// $Id: breakpoint_mgr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <vector>
#include "zdk/mutex.h"
#include "zdk/zobject_impl.h"
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "breakpoint.h"
#include "dbgout.h"

class DebuggerEngine;
class WatchPoint;

typedef void (DebuggerEngine::*Callback)(volatile BreakPoint*);


/**
 * BreakPointManager implementation
 */
CLASS BreakPointManagerImpl : public BreakPointManagerBase
                            , public EnumCallback<volatile BreakPoint*>
{
public:
    typedef std::vector<RefPtr<BreakPointBase> > BreakPointList;

    // breakpoints are internally keyed by their addresses
    typedef ext::hash_map<addr_t, RefPtr<BreakPointBase> > BreakPointMap;
    typedef ext::hash_map<pid_t, BreakPointMap> PerThreadBreakPoints;

    BreakPointManagerImpl
    (
        RefPtr<Process>,
        int  verbose,       // verbosity level
        bool useHardware,   // use hardware support?
        Callback onInsert,
        Callback onRemove
    );

    virtual ~BreakPointManagerImpl() throw();

    DebugChannel debug_channel(const char* fn) const
    {
        return DebugChannel(fn, verbose_);
    }

    ///// Notifications that let the breakpoint manager know
    ///// when threads are created and when they exit; there
    ///// are per-thread containers of break points that need
    ///// to be created/updated and destroyed, respectively.

    bool on_thread_created(Thread&);

    void on_thread_unmanage(Thread&);

    int verbose() const throw() { return verbose_; }

    bool use_hardware() const throw() { return useHardware_; }

    void set_use_hardware(bool flag) { useHardware_ = true; }

    RefPtr<BreakPoint>
    get_breakpoint(const Process&, addr_t, const Thread*) const;

    RefPtr<BreakPoint>
    get_breakpoint(const Thread& thread, addr_t addr, const Thread* = 0) const
    {
        return get_breakpoint_impl(thread, addr);
    }

    template<typename T>
    size_t get_breakpoint_list(const T& t, BreakPointList& v) const
    {
        size_t size = 0;

        if (const BreakPointMap* mptr = get_breakpoint_map(t))
        {
            v.reserve(size = mptr->size());
            for (BreakPointMap::const_iterator i = mptr->begin();
                    i != mptr->end();
                    ++i)
            {
                v.push_back(i->second);
            }
        }
        return size;
    }

    /**
     * Lookup a watchpoint by address and pid
     */
    RefPtr<WatchPoint> get_watchpoint(Thread*, addr_t, WatchType) const;

    void erase(RefPtr<BreakPoint>);

    void activate(RefPtr<BreakPoint>, const SymbolTable&);

    virtual BreakPointBase* set_breakpoint
      (
        Runnable*,
        BreakPoint::Type,
        addr_t,
        BreakPoint::Action*,
        bool deferred = false,
        Symbol* = NULL
      );

    virtual BreakPoint* set_watchpoint
      (
        Runnable* thread,
        WatchType type,
        bool global,
        addr_t addr,
        BreakPoint::Action*
      );

    /**
     * This method enumerates break points by thread,
     * address and type. If thread is null, any global or
     * per-thread break point matches; if address is
     * not zero, only break points that match the
     * address are enumerated; if type is ANY all match.
     *
     * @return the number of matching breakpoints.
     */
    virtual size_t enum_breakpoints
      (
        EnumCallback<volatile BreakPoint*>*,
        Thread* = 0,
        addr_t = 0,
        BreakPoint::Type = BreakPoint::ANY
      ) const;

    virtual size_t enum_watchpoints
      (
        EnumCallback<volatile BreakPoint*>*
      ) const;

    // calls clone_breakpoint
    virtual void notify(volatile BreakPoint*);

    size_t remove_breakpoint_actions(pid_t, pid_t, addr_t, const char*);

    /**
     * helper for remove_breakpoint_actions
     */
    size_t remove_actions
      (
        BreakPointMap&,
        BreakPointMap::iterator&,
        const char* actionName
      );

    /**
     * helper for remove_breakpoint_actions
     */
    size_t remove_actions(BreakPointMap&, addr_t, const char* name);

private:
    void clone_breakpoint(volatile BreakPoint*);

    /**
     * Helper function; for all break points in the given map
     * that match the address and the type, call the callback
     * interface (if not null); returns the number of matching
     * break points.
     * @note a zero address matches all, ditto for BreakPoint::ANY
     * @note the breakpoint map is passed in by value on purpose;
     * the iteration is done over a copy of the map object, so we
     * can safely modify the map from within the EnumCallback.
     */
    static size_t enum_breakpoints
      (
        BreakPointMap,
        addr_t,
        BreakPoint::Type,
        EnumCallback<volatile BreakPoint*>*
      );

    /**
     * Enumerate deferred breakpoints
     */
    size_t enum_deferred
      (
        Thread*,
        addr_t,
        BreakPoint::Type,
        EnumCallback<volatile BreakPoint*>*
      ) const;

    /**
     * lookup deferred breakpoint
     */
    RefPtr<BreakPointBase> find_deferred
      (
        Thread*,
        BreakPoint::Type,
        addr_t
      ) const;

    void call_callback(Callback, volatile BreakPoint*);

    /**
     * @return the number of hardware breakpoints used by
     * thread of given id
     */
    unsigned int hardware_breakpoint_count(pid_t tid) const;

    /**
     * Wrapper around the DebugRegs::set_breakpoint method.
     * Checks that there are hardware breakpoints available
     * before trying to set a new one; the reason is that
     * the debug registers don't know whether a hardware
     * breakpoint is in use but temporarily desabled; in this
     * case, it would appear that a CPU debug register is
     * available, when in reality it is assigned.
     */
    bool set_hardware_breakpoint
      (
        Thread& thread,
        addr_t,
        uint32_t*,
        bool global = false,
        HardwareBreakPoint::Condition = DebugRegs::BREAK_ON_INSTRUCTION,
        HardwareBreakPoint::Length = DebugRegs::BREAK_ONE_BYTE
      );

    const BreakPointMap* get_breakpoint_map(const Thread&) const;

    const BreakPointMap* get_breakpoint_map(const Process& proc) const
    {
        assert(proc.pid());
        if (proc.pid() == pid())
        {
            return &globalBreakPoints_;
        }
        return NULL;
    }

    Thread* get_thread(Runnable*) const;

    RefPtr<BreakPointBase> find_breakpoint
      (
        Thread*,
        BreakPoint::Type,
        addr_t,
        BreakPointMap*&
      );

    RefPtr<BreakPointBase> create_breakpoint
      (
        Thread*,
        BreakPoint::Type,
        addr_t
      );

    template<typename T>
    RefPtr<BreakPoint> get_breakpoint_impl(const T& t, addr_t addr) const
    {
        RefPtr<BreakPoint> result;
        if (const BreakPointMap* mptr = this->get_breakpoint_map(t))
        {
            BreakPointMap::const_iterator i = mptr->find(addr);
            if (i != mptr->end())
            {
                result = i->second;
            }
        }
        return result;
    }

private:
    mutable Mutex mutex_;
    PerThreadBreakPoints perThreadBreakPoints_;

    BreakPointMap globalBreakPoints_;

    std::vector<RefPtr<WatchPoint> > watchPoints_;

    BreakPointList deferred_;

    mutable int verbose_;   // verbosity level, for debug
    bool useHardware_;      // use hardware break points?

    Callback onInsert_;
    Callback onRemove_;
};



/**
 * This class is here because I got the initial design wrong --
 * I did not have the notion of a group of threads (i.e. Process)
 *
 * All threads in a process share the software breakpoints
 * (since they share the .text segment); but when a new process
 * if forked, the software breakpoints need to be replicated
 * into that process code, and managed separately. When I
 * realized this, the easiest fix I could come up with was to
 * take the BreakPointManagerImpl class that I already had,
 * create a new class (this one) with roughly the same interface
 * and then have it aggregate a BreakPointManagerImpl instance
 * for each forked process. The client code did not have to
 * change, and the implementation of this class is straight-forward:
 * it forwards the work to one of the managers (identified
 * by the pid of the process group lead).
 */
CLASS BreakPointManagerGroup : public BreakPointManagerBase
{
public:
    BreakPointManagerGroup(RefPtr<Process>, int, bool, Callback, Callback);

    virtual ~BreakPointManagerGroup() throw();

    DebugChannel debug_channel(const char* fn) const
    {
        return DebugChannel(fn, verbose_);
    }

    ///// Notifications that let the break pointmanager know
    ///// when threads are created and when they exit; there
    ///// are per-thread containers of break points that need
    ///// to be created/updated and destroyed, respectively.

    bool on_thread_created(Thread&);

    void on_thread_unmanage(Thread&);

    void on_exec(Thread&);

    int verbose() const throw() { return verbose_; }

    bool use_hardware() const throw() { return useHardware_; }

    void set_use_hardware(bool flag) { useHardware_ = true; }

    RefPtr<WatchPoint> get_watchpoint(Thread*, addr_t, WatchType) const;


    template<typename T>
    RefPtr<BreakPoint> get_breakpoint(const T& t, addr_t addr, const Thread* owner = 0) const
    {
        RefPtr<BreakPoint> result;
        Group::const_iterator i = group_.begin();
        for (; i != group_.end() && !result; ++i)
        {
            result = (*i)->get_breakpoint(t, addr, owner);
        }
        return result;
    }

/*
    template<typename T> size_t get_breakpoint_list
    (
        const T& t,
        addr_t addr,
        std::vector<RefPtr<BreakPoint> >& brkpnts
    ) const
    {
        size_t result = 0;

        Group::const_iterator i = group_.begin();
        for (; i != group_.end(); ++i)
        {
            if (addr)
            {
                RefPtr<BreakPoint> bpnt = (*i)->get_breakpoint(t, addr);

                if (bpnt.get())
                {
                    brkpnts.push_back(bpnt);
                    ++result;
                }
            }
            else
            {
                result += (*i)->get_breakpoint_list(t, brkpnts);
            }
        }
        return result;
    }
 */
    void erase(RefPtr<BreakPoint>);

    /**
     * Activate deferred breakpoint
     */
    void activate(RefPtr<BreakPoint>, const SymbolTable&);

    virtual BreakPointBase* set_breakpoint(
        Runnable*,
        BreakPoint::Type,
        addr_t,
        BreakPoint::Action*,
        bool deferred,
        Symbol*);

    virtual BreakPoint* set_watchpoint(
        Runnable* thread,
        WatchType type,
        bool global,
        addr_t addr,
        BreakPoint::Action*);

    /**
     * This method enumerates breakpoints by thread,
     * address and type. If thread is null, any global or
     * per-thread breakpoint matches; if address is
     * not zero, only break points that match the
     * address are enumerated; if type is ANY all match.
     *
     * @return the number of matching break points.
     */
    virtual size_t enum_breakpoints(
        EnumCallback<volatile BreakPoint*>*,
        Thread* = 0,
        addr_t = 0,
        BreakPoint::Type = BreakPoint::ANY) const;

    virtual size_t enum_watchpoints(
        EnumCallback<volatile BreakPoint*>*) const;

    size_t remove_breakpoint_actions(pid_t, pid_t, addr_t, const char*);

    BreakPointManager* get_manager(pid_t) const;

private:
    typedef std::vector<RefPtr<BreakPointManagerImpl> > Group;

    void replicate_breakpoints(Thread&, BreakPointManagerImpl&);

    RefPtr<Process> process_;
    mutable int verbose_;      // verbosity level, for debug
    bool        useHardware_;  // use hardware break points?
    Callback    onInsert_;
    Callback    onRemove_;
    Group       group_;
};
#endif // BREAKPOINT_MGR_H__9A0FCE8C_CBCC_4709_A0EC_694971AF6575
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
