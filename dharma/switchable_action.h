#ifndef SWITCHABLE_ACTION_H__6B5C3CD7_16F0_41CC_80F2_E00C1EF48654
#define SWITCHABLE_ACTION_H__6B5C3CD7_16F0_41CC_80F2_E00C1EF48654
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

#include "zdk/breakpoint.h"
#include "zdk/switchable.h"

struct Variant;


CLASS SwitchableAction : public ZObjectImpl<BreakPointAction>, public Switchable
{
public:

BEGIN_INTERFACE_MAP(SwitchableAction)
    INTERFACE_ENTRY(BreakPointAction)
    INTERFACE_ENTRY(Switchable)
END_INTERFACE_MAP()

    SwitchableAction(const std::string&, bool = true, word_t = 0);

    virtual ~SwitchableAction() throw();

    void execute_(Thread*, BreakPoint*);

    bool pending() const;

protected:
    class EvalEvents;

    // --- BreakPointAction interface -------------------------
    const char* name() const { return name_.c_str(); }

    bool execute(Thread*, BreakPoint*);

    word_t cookie() const { return cookie_; }

    // --- Switchable interface --------------------------------
    bool is_enabled() const volatile
    {
        return atomic_read(count_) > 0;
    }

    void enable() volatile { atomic_inc(count_); }

    void disable() volatile { atomic_dec(count_); }

    unsigned long counter() const { return hits_; }

    void set_counter(unsigned long hits) { hits_ = hits; }

    unsigned long activation_counter() const { return threshold_; }

    void set_activation_counter(unsigned long threshold, bool reset)
    {
        threshold_ = threshold;
        autoReset_ = reset;
    }

    bool auto_reset() const { return autoReset_; }

    const char* activation_expr() const { return expr_.c_str(); }

    void set_activation_expr(const char* expr)
    {
        if (expr)
        {
            expr_.assign(expr);
        }
        else
        {
            expr_.clear();
        }
    }

    enum Condition
    {
        COND_FALSE,
        COND_TRUE,
        COND_PENDING
    };

    Condition eval_condition(Debugger&, Thread*, BreakPoint*);

    virtual void execute_impl(Debugger&, Thread*, BreakPoint*) = 0;

private:
    std::string     name_;
    bool            keep_;
    bool            autoReset_;
    bool            pending_;
    word_t          cookie_;
    atomic_t        count_;
    unsigned long   hits_;
    unsigned long   threshold_;
    std::string     expr_;
    RefPtr<Variant> evalResult_;
};
#endif // SWITCHABLE_ACTION_H__6B5C3CD7_16F0_41CC_80F2_E00C1EF48654
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
