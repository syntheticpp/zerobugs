#ifndef BREAKPOINT_H__9F2DD290_B8AF_4C66_BAFC_D9AC15A76050
#define BREAKPOINT_H__9F2DD290_B8AF_4C66_BAFC_D9AC15A76050
//
// $Id: breakpoint.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iosfwd>
#include <map>
#include <vector>
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "zdk/zero.h"
#include "zdk/weak_ptr.h"
#include "zdk/zobject_impl.h"


ZDK_LOCAL std::ostream& operator<<(std::ostream& outs, BreakPoint::Type);

/**
 * Several software breakpoints might share the same address.
 */
struct ZDK_LOCAL PhysicalBreakPoint
{
    word_t  origCode;
    bool    enabled;

    PhysicalBreakPoint() : origCode(0), enabled(false) { }
};


/**
 * Implementation shared by BreakPointManagerImpl
 * and BreakPointManagerGroup.
 */
CLASS BreakPointManagerBase : public ZObjectImpl<BreakPointManager>
{
protected:
    BreakPointManagerBase(RefPtr<Process> p) : process_(p)
    { }

    ~BreakPointManagerBase() throw() { }

public:
    typedef ext::hash_map<addr_t, PhysicalBreakPoint> PhysicalMap;

    bool enable_software_breakpoint(Thread&, addr_t, bool);

    pid_t pid() const volatile { return process_ ? process_->pid() : 0; }

    Process* process() const { return process_.get(); }

    const PhysicalMap& physical_map() const { return map_; }

    void set_physical_map(const PhysicalMap& m) { map_ = m; }

private:
    RefPtr<Process> process_;
    PhysicalMap map_;
};


/**
 * Partial implementation of the BreakPoint interface,
 * manages the actions that are executed when the
 * breakpoint is hit.
 */
CLASS BreakPointBase : public RefCountedImpl<BreakPoint>
{
public:
    typedef std::vector<RefPtr<Action> > ActionList;

    // map actions to action contexts
    typedef std::map<Action*, RefPtr<ZObject> > ActionMap;

    DECLARE_UUID("abc4b693-c705-4dc3-899f-cd7aa2bae625")

    BEGIN_INTERFACE_MAP(BreakPointBase)
        INTERFACE_ENTRY(BreakPointBase)
        INTERFACE_ENTRY(BreakPoint)
    END_INTERFACE_MAP()

    virtual ~BreakPointBase() throw();

    virtual bool is_enabled() const volatile;

    virtual bool is_deferred() const volatile { return false; }

    /**
     * Temporarily turn the breakpoint on/off (ZDK)
     */
    virtual int enable() volatile;

    virtual int disable() volatile;

    virtual bool add_action(Action*);

    /**
     * Execute all actions associated with this breakpoint.
     */
    virtual void execute_actions(Thread*);

public:
    /**
     * Prints the actions associated with this breakpoint
     * to an output stream
     */
    void print(std::ostream&) const volatile;

    /**
     * the number of actions associated with this breakpoint
     */
    size_t action_count() const volatile;

    const Action* action(size_t) const volatile;

    virtual Thread* thread() const volatile;

    virtual addr_t addr() const volatile { return addr_; }

    virtual Symbol* symbol() const volatile;

    virtual size_t enum_actions(
        const char* name = NULL,
        EnumCallback<Action*>* = NULL) const volatile;

    virtual size_t remove_actions(const char* = NULL) volatile;

    virtual size_t remove(Action*, word_t) volatile;

    virtual ZObject* action_context(Action*) const;

    const ActionList& actions() const { return actions_; }

    void set_symbol(Symbol* sym) { sym_ = sym; }

    void append(const ActionList& actions)
    {
        actions_.insert(actions_.end(),
                        actions.begin(),
                        actions.end());
    }

    /**
     * called by the BreakPointMgr
     */
    virtual BreakPointBase* clone(BreakPointManagerBase*, RefPtr<Thread>) = 0;

protected:
    BreakPointBase(RefPtr<Thread>, addr_t, int enabled = 0);

    BreakPointBase(RefPtr<Thread>, RefPtr<Symbol>, addr_t);

    BreakPointBase(const BreakPointBase&, Thread&);

    virtual bool do_enable(bool) volatile = 0;

    void error(bool, const char* = 0) const volatile;

    void debug_enable(bool, Thread* = NULL) volatile;

    void reparent(const RefPtr<Thread>& thread) volatile;

    RefPtr<Thread> thread_ptr() const { return thread_; }

private:
    RefPtr<Thread>          thread_; // owner
    mutable RefPtr<Symbol>  sym_;
    addr_t                  addr_;
    int                     enable_; // > 0 --> enabled
    ActionList              actions_;
    ActionMap               actionContexts_;
};



/**
 * This class models a software breakpoint
 */
CLASS SoftwareBreakPoint : public BreakPointBase
{
public:
    DECLARE_UUID("6c8dbdd5-1691-4bee-8b57-5a8255a921fe")

    BEGIN_INTERFACE_MAP(SoftwareBreakPoint)
        INTERFACE_ENTRY(SoftwareBreakPoint)
        INTERFACE_ENTRY_INHERIT(BreakPointBase)
    END_INTERFACE_MAP()

    SoftwareBreakPoint(BreakPointManagerBase*, RefPtr<Thread>, addr_t);

    ~SoftwareBreakPoint() throw();

    virtual Type type() const volatile { return SOFTWARE; }

protected:
    virtual bool do_enable(bool onOff) volatile;

    SoftwareBreakPoint(
            const SoftwareBreakPoint&,
            BreakPointManagerBase*,
            Thread&);

    virtual BreakPointBase* clone(BreakPointManagerBase*, RefPtr<Thread>);

    virtual bool is_enabled() const volatile;

private:
    WeakPtr<BreakPointManagerBase> mgr_;
};



/**
 * Emulates the behavior of a hardware breakpoint with a
 * software breakpoint. Normally, software breakpoints
 * cause any thread that reach them to get a SIGTRAP,
 * whereas hardware breaks apply only to a given pid.
 */
CLASS EmulatedBreakPoint : public SoftwareBreakPoint
{
public:
    DECLARE_UUID("04164e56-bd2f-4c56-a544-4900a08e8b89")

    BEGIN_INTERFACE_MAP(EmulatedBreakPoint)
        INTERFACE_ENTRY(EmulatedBreakPoint)
        INTERFACE_ENTRY_INHERIT(SoftwareBreakPoint)
    END_INTERFACE_MAP()

    EmulatedBreakPoint(BreakPointManagerBase*, RefPtr<Thread>, addr_t addr);

protected:
    EmulatedBreakPoint
    (
        const EmulatedBreakPoint& bpnt,
        BreakPointManagerBase* mgrBase,
        Thread& thread
    )
      : SoftwareBreakPoint(bpnt, mgrBase, thread)
    { }

    virtual void execute_actions(Thread*);

    virtual Type type() const volatile { return EMULATED; }

    virtual BreakPointBase* clone(BreakPointManagerBase*, RefPtr<Thread>);
};


/**
 * Models a hardware breakpoint.
 * @see DebugRegs, DebugRegs386
 */
CLASS HardwareBreakPoint : public BreakPointBase
{
public:
    typedef DebugRegs::Condition Condition;
    typedef DebugRegs::Length Length;

    DECLARE_UUID("cf697495-5a63-4bc7-a950-07464d8e8088")

    BEGIN_INTERFACE_MAP(HardwareBreakPoint)
        INTERFACE_ENTRY(HardwareBreakPoint)
        INTERFACE_ENTRY_INHERIT(BreakPointBase)
    END_INTERFACE_MAP()

    /**
     * @param thread
     * @param address of breakpoint
     * @param debugReg the index of the debug register to use
     * @param global if true, applies to all threads
     */
    HardwareBreakPoint(
        RefPtr<Thread>,
        addr_t,
        int debugReg,
        bool global = false,
        Condition = DebugRegs::BREAK_ON_INSTRUCTION,
        Length = DebugRegs::BREAK_ONE_BYTE);

    ~HardwareBreakPoint() throw();

    virtual Type type() const volatile { return HARDWARE; }

    /**
     * The Intel processor has 4 debug registers that can
     * be used for setting hardware breakpoints.
     * Return the index of the register this breakpoint
     * is using.
     * @see DebugRegs386
     */
    int debug_register() const volatile { return register_; }

    bool is_global() const volatile { return global_; }

    Condition condition() const volatile { return cond_; }

    virtual void execute_actions(Thread*);

protected:
    HardwareBreakPoint( bool global,
                        RefPtr<Thread>,
                        addr_t,
                        int debugReg, // index of debug reg to use
                        Condition
                      );

    virtual bool do_enable(bool onOff) volatile;

    bool do_enable(Thread&, bool onOff, std::string& errMsg) volatile throw();

    /**
     * Cannot clone hardware breakpoints, since we cannot
     * clone the underlying hardware registers.
     */
    virtual BreakPointBase* clone(BreakPointManagerBase*, RefPtr<Thread>);

private:
    int         register_;  // which hw debug register
    bool        global_;    // applies to all threads?
    Condition   cond_;
    Length      len_;       // length in bytes, if memory breakpnt
};


/**
 * For inserting breapoints inside shared objects that
 * have not yet been loaded (mapped) into the target's
 * memory
 */
CLASS DeferredBreakPoint : public BreakPointBase
{
public:
    DECLARE_UUID("08fa5b89-cb4f-4546-8df6-e2b6c963f80d")

    BEGIN_INTERFACE_MAP(DeferredBreakPoint)
        INTERFACE_ENTRY(DeferredBreakPoint)
        INTERFACE_ENTRY_INHERIT(BreakPointBase)
    END_INTERFACE_MAP()

    DeferredBreakPoint
      (
        BreakPointManagerBase*,
        RefPtr<Thread>,
        Type,
        addr_t
      );

    ~DeferredBreakPoint() throw();

    Type type() const volatile { return type_; }

protected:
    bool do_enable(bool onOff) volatile;

    DeferredBreakPoint(
            const DeferredBreakPoint&,
            BreakPointManagerBase*,
            Thread&);

    BreakPointBase* clone(BreakPointManagerBase*, RefPtr<Thread>);

    void execute_actions(Thread*);

    bool is_deferred() const volatile { return true; }

private:
    WeakPtr<BreakPointManagerBase> mgr_;
    Type type_;
};


inline ZDK_LOCAL std::ostream&
operator<<(std::ostream& out, const volatile BreakPointBase& b)
{
    b.print(out);
    return out;
}

#endif // BREAKPOINT_H__9F2DD290_B8AF_4C66_BAFC_D9AC15A76050
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
