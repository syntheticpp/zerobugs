#ifndef WATCHPOINT_H__D15518E6_3185_4E66_A88B_3F3E5CF6CB71
#define WATCHPOINT_H__D15518E6_3185_4E66_A88B_3F3E5CF6CB71
//
// $Id: watchpoint.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "breakpoint.h"
#include "dharma/switchable_action.h"
#include "zdk/watchtype.h"


struct DebugSymbol;


/**
 * Memory watch-points are hardware breakpoints that are
 * activated by a memory access.
 */
CLASS WatchPoint : public HardwareBreakPoint
{
public:
    DECLARE_UUID("22a9da60-340c-4dbe-8366-d8f9411a91b6")

    BEGIN_INTERFACE_MAP(WatchPoint)
        INTERFACE_ENTRY(WatchPoint)
        INTERFACE_ENTRY(BreakPoint)
        INTERFACE_ENTRY(HardwareBreakPoint)
    END_INTERFACE_MAP()

    WatchPoint( RefPtr<Thread>,
                WatchType,
                addr_t,
                int,
                bool global,
                Condition );

    ~WatchPoint() throw();

    WatchType watch_type() const { return type_; }

    bool in_scope(Thread* thread) const;

    void set_scope(addr_t scope)
    {
        assert(scope_ == 0); // set once
        scope_ = scope;
    }

private:
    WatchType type_;
    addr_t scope_;
};


/* Actions associated with watchpoints */
/**
 * Action is triggered when memory location is being accessed.
 */
CLASS MemoryWatch : public SwitchableAction
                  , public BreakPointAction::Info
{
public:
BEGIN_INTERFACE_MAP(MemoryWatch)
    INTERFACE_ENTRY_INHERIT(SwitchableAction)
    INTERFACE_ENTRY(BreakPointAction::Info)
END_INTERFACE_MAP()

    MemoryWatch();

    ~MemoryWatch() throw();

    void set_owner(WatchPoint& watchPoint)
    {
        assert(watchPoint_ == 0);
        watchPoint_ = &watchPoint;
    }

protected:
    virtual const char* description() const;

    virtual bool is_global() const;

    virtual std::string description_impl() const;

    virtual void execute_impl(Debugger&, Thread*, BreakPoint*);

    bool in_scope(Thread*) const;

private:
    WatchPoint* watchPoint_;
    std::string description_;
};


/**
 * Action is triggered when specified value is stored
 * at a given memory location.
 */
CLASS ValueWatch : public MemoryWatch
{
public:
    ValueWatch(
        RefPtr<DebugSymbol>,/* symbol to watch */
        RelType,            /* equal, lower than, etc. */
        RefPtr<SharedString> val);

    ~ValueWatch() throw();

protected:
    virtual const char* name() const;

    virtual word_t cookie() const;

    void execute_impl(Debugger&, Thread*, BreakPoint*);

    virtual std::string description_impl() const;

private:
    RefPtr<DebugSymbol>     sym_;
    RelType                 rel_;
    RefPtr<SharedString>    val_;
};

#endif // WATCHPOINT_H__D15518E6_3185_4E66_A88B_3F3E5CF6CB71
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
