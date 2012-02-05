#include "zdk/zero.h"
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
#include "zdk/check_ptr.h"
#include "zdk/interface_cast.h"
#include "zdk/switchable.h"
#include "zdk/breakpoint_util.h"

enum Operation
{
    ACTION_QUERY_ENABLED,
    ACTION_QUERY_DISABLED,
    ACTION_QUERY_SWITCHABLE,
    ACTION_QUERY_CONDITIONAL,
    ACTION_ENABLE,
    ACTION_DISABLE,
};


////////////////////////////////////////////////////////////////
class Action_Helper : public EnumCallback<BreakPoint::Action*>
{
public:
    Action_Helper(volatile BreakPoint*, Operation operation)
        : operation_(operation), result_(false)
    {}

    bool result() const { return result_; }

private:
    void notify(BreakPoint::Action* action)
    {
        if (Switchable* sw = interface_cast<Switchable*>(action))
        {
            switch (operation_)
            {
            case ACTION_QUERY_SWITCHABLE:
                result_ = true;
                break;

            case ACTION_QUERY_ENABLED:
                // have at least one enabled action?
                if (sw->is_enabled())
                {
                    result_ = true;
                }
                break;

            case ACTION_QUERY_DISABLED:
                // have at least one disabled action?
                if (!sw->is_enabled())
                {
                    result_ = true;
                }
                break;

            case ACTION_QUERY_CONDITIONAL:
                if (sw->activation_counter()
                 || *CHKPTR(sw->activation_expr()))
                {
                    result_ = true;
                }
                break;

            case ACTION_ENABLE:
                if (!sw->is_enabled())
                {
                    sw->enable();
                    result_ = true;
                }
                break;

            case ACTION_DISABLE:
                if (sw->is_enabled())
                {
                    sw->disable();
                    result_ = true;
                }
                break;
            }
        }
    }

private:
    Operation   operation_;
    bool        result_;
};


////////////////////////////////////////////////////////////////
class BreakPoint_Helper : public EnumCallback<volatile BreakPoint*>
{
public:
    explicit BreakPoint_Helper(Operation oper, const char* action = NULL)
        : operation_(oper), result_(false), action_(action)
    {}

    bool result() const { return result_; }

private:
    void notify(volatile BreakPoint* breakpoint)
    {
        Action_Helper helper(breakpoint, operation_);

        breakpoint->enum_actions(action_, &helper);

        result_ |= helper.result();
    }

private:
    Operation   operation_;
    bool        result_;
    const char* action_;
};



////////////////////////////////////////////////////////////////
class Switchable_Helper
    : public EnumCallback<volatile BreakPoint*>
    , public EnumCallback<BreakPoint::Action*>
{
public:
    Switchable_Helper(const std::string& cond, size_t count, bool autoReset)
        : condition_(cond), count_(count), autoReset_(autoReset)
    { }

    virtual ~Switchable_Helper() throw() { }

    void notify(volatile BreakPoint* bpnt)
    {
        if (bpnt)
        {
            bpnt->enum_actions("USER", this);
        }
    }

    void notify(BreakPoint::Action* action)
    {
        if (Switchable* sa = interface_cast<Switchable*>(action))
        {
            sa->set_activation_expr(condition_.c_str());
            sa->set_activation_counter(count_, autoReset_);
        }
    }

private:
    std::string condition_;
    size_t      count_;
    bool        autoReset_;
};


bool has_switchable_actions(volatile BreakPoint& bpnt)
{
    Action_Helper helper(&bpnt, ACTION_QUERY_SWITCHABLE);
    bpnt.enum_actions(NULL, &helper);

    return helper.result();
}


bool has_disabled_actions(volatile BreakPoint& bpnt)
{
    Action_Helper helper(&bpnt, ACTION_QUERY_DISABLED);
    bpnt.enum_actions(NULL, &helper);
    return helper.result();
}


bool has_enabled_actions(volatile BreakPoint& bpnt)
{
    Action_Helper helper(&bpnt, ACTION_QUERY_ENABLED);
    bpnt.enum_actions(NULL, &helper);
    return helper.result();
}


bool enable_actions(volatile BreakPoint& bpnt)
{
    Action_Helper helper(&bpnt, ACTION_ENABLE);
    // enumerate ALL actions
    bpnt.enum_actions(NULL, &helper);

    return helper.result();
}


bool enable_user_actions(volatile BreakPoint& bpnt)
{
    Action_Helper helper(&bpnt, ACTION_ENABLE);
    bpnt.enum_actions("USER", &helper);

    return helper.result();
}


/**
 * Determine whether there are any enabled breakpoint
 * actions set at the specified address.
 */
bool has_enabled_user_breakpoint_actions
(
    Debugger& dbg,
    addr_t addr,
    Thread* thread
)
{
    BreakPoint_Helper helper(ACTION_QUERY_ENABLED, "USER");
    if (BreakPointManager* bpntMgr = dbg.breakpoint_manager())
    {
        bpntMgr->enum_breakpoints(&helper, thread, addr);
    }
    return helper.result();
}


/**
 * Determine if there are any disabled breakpoint
 * actions set at the specified address.
 * @todo rename to has_disabled_user_breakpoint_actions
 */
bool has_disabled_user_breakpoint_actions
(
    Debugger& dbg,
    addr_t addr,
    Thread* thread
)
{
    BreakPoint_Helper helper(ACTION_QUERY_DISABLED, "USER");
    if (BreakPointManager* bpntMgr = dbg.breakpoint_manager())
    {
        bpntMgr->enum_breakpoints(&helper, thread, addr);
    }
    return helper.result();
}


/**
 * Enable all global breakpoint actions at address.
 * If there are no disabled breakpoint actions, nothin happens.
 */
void enable_breakpoint_actions(Debugger& dbg, addr_t addr)
{
    if (BreakPointManager* bpntMgr = dbg.breakpoint_manager())
    {
        BreakPoint_Helper helper(ACTION_ENABLE);
        bpntMgr->enum_breakpoints(&helper, NULL, addr);
    }
}


void disable_breakpoint_actions(Debugger& dbg, addr_t addr)
{
    if (BreakPointManager* bpntMgr = dbg.breakpoint_manager())
    {
        BreakPoint_Helper helper(ACTION_DISABLE);
        bpntMgr->enum_breakpoints(&helper, NULL, addr);
    }
}


void
enable_user_breakpoint_actions(Debugger& dbg, addr_t addr, Thread* thread)
{
    if (BreakPointManager* bpntMgr = dbg.breakpoint_manager())
    {
        BreakPoint_Helper helper(ACTION_ENABLE, "USER");
        bpntMgr->enum_breakpoints(&helper, thread, addr);
    }
}


void
disable_user_breakpoint_actions(Debugger& dbg, addr_t addr, Thread* thread)
{
    if (BreakPointManager* bpntMgr = dbg.breakpoint_manager())
    {
        BreakPoint_Helper helper(ACTION_DISABLE, "USER");
        bpntMgr->enum_breakpoints(&helper, thread, addr);
    }
}


void set_breakpoint_conditions
(
    Debugger&           dbg,
    Platform::addr_t    addr,
    const std::string&  cond,
    unsigned long       activationCount,
    bool                autoReset
)
{
    if (BreakPointManager* bpntMgr = dbg.breakpoint_manager())
    {
        Switchable_Helper helper(cond, activationCount, autoReset);
        bpntMgr->enum_breakpoints(&helper, NULL, addr);
    }
}


// todo: rename to has_conditional_user_actions
bool has_conditional_actions(Debugger& dbg, addr_t addr)
{
    BreakPoint_Helper helper(ACTION_QUERY_CONDITIONAL, "USER");
    if (BreakPointManager* bpntMgr = dbg.breakpoint_manager())
    {
        bpntMgr->enum_breakpoints(&helper, NULL, addr);
    }
    return helper.result();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
