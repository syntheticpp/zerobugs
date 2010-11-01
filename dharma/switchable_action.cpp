//
// $Id: switchable_action.cpp 710 2010-10-16 07:09:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include <signal.h>
#include "zdk/expr.h"
#include "zdk/check_ptr.h"
#include "zdk/observer_impl.h"
#include "zdk/shared_string_impl.h"
#include "zdk/thread_util.h"
#include "zdk/variant_util.h"
#include "interp/variant_impl.h"
#include "switchable_action.h"

using namespace std;


/**
 * Observer receives events during expression evaluation.
 *
 * @note a SwitchableAction can be conditional, i.e.  executed only
 * when a given expression evaluates to non-zero.
 */
class SwitchableAction::EvalEvents : public SubjectImpl<ExprEvents>
{
public:
    EvalEvents(Thread*, BreakPoint*, SwitchableAction*, Variant*);

    virtual ~EvalEvents() throw() { }

private:
    EvalEvents(const EvalEvents& other)
        : thread_(other.thread_)
        , bpnt_(other.bpnt_)
        , action_(other.action_)
        , result_(other.result_)
        , callRetAddr_(other.callRetAddr_)
    { }

    bool on_done(Variant*, bool*, DebugSymbolEvents*);

    void on_error(const char*);

    void on_warning(const char*) { };

    bool on_event(Thread*, addr_t);

    void on_call(addr_t, Symbol*);

    ExprEvents* clone() const
    {
        return new EvalEvents(*this);
    }

private:
    RefPtr<Thread>              thread_;
    RefPtr<BreakPoint>          bpnt_; // keeps the bpnt alive
    RefPtr<SwitchableAction>    action_;
    RefPtr<Variant>             result_;
    addr_t                      callRetAddr_;
};


////////////////////////////////////////////////////////////////
SwitchableAction::SwitchableAction
(
    const string&   name,
    bool            keep,
    word_t          cookie
)
  : name_(name)
  , keep_(keep)
  , autoReset_(false)
  , pending_(false)
  , cookie_(cookie)
  , count_(1)
  , hits_(0)
  , threshold_(0)
{
}


////////////////////////////////////////////////////////////////
SwitchableAction::~SwitchableAction() throw()
{
}


////////////////////////////////////////////////////////////////
bool SwitchableAction::pending() const
{
    return pending_;
}


////////////////////////////////////////////////////////////////
bool SwitchableAction::execute(Thread* thread, BreakPoint* bpnt)
{
    assert(bpnt);
    assert(thread);

    if (!pending_)
    {
        execute_(thread, bpnt);
    }
    if (!evalResult_)
    {
        pending_ = false;
    }
    return keep_;
}


////////////////////////////////////////////////////////////////
void SwitchableAction::execute_(Thread* thread, BreakPoint* bpnt)
{
    assert(bpnt);
    assert(thread);

    Debugger* dbg = CHKPTR(thread->debugger());
    if (!dbg)
    {
        return;
    }
    // make sure that the debugger is attached to the thread
    assert(thread_is_attached(*thread));

    switch (eval_condition(*dbg, thread, bpnt))
    {
    case COND_TRUE:
        ++hits_;
        if ((threshold_ == 0) || (threshold_ <= hits_))
        {
            execute_impl(*dbg, thread, bpnt);

            if (autoReset_)
            {
                hits_ = 0;
            }
        }
        // fallthru

    case COND_FALSE:
        // execute() takes care of reseting the
        // pending_ flag on its way out
        assert(!pending_);
        break;

    case COND_PENDING:
        //bpnt->disable();
        pending_ = true; // defer execution until on_done notification
        break;
    }
}


////////////////////////////////////////////////////////////////
SwitchableAction::Condition
SwitchableAction::eval_condition(Debugger&       eng,
                                 Thread*         thread,
                                 BreakPoint*     bpnt)
{
    if (expr_.empty())
    {
        return COND_TRUE;
    }
#ifdef DEBUG
    clog << __func__ << ": " << expr_ << endl;
#endif

    if (!evalResult_)
    {
        // create a variant for the expression's result
        evalResult_ = new VariantImpl;

        RefPtr<EvalEvents> events =
            new EvalEvents(thread, bpnt, this, evalResult_.get());

        const addr_t addr = CHKPTR(bpnt)->addr();

        // evaluate condition
        if (!eng.evaluate(expr_.c_str(), thread, addr, events.get()))
        {
            return COND_PENDING;
        }
    }
#ifdef DEBUG
    assert(evalResult_);
    variant_print(clog << __func__ << "=", *evalResult_) << endl;
#endif
    Condition c = COND_FALSE;

    if (variant_true(*evalResult_))
    {
        c = COND_TRUE;
        RefPtr<SharedString> str = shared_string(expr_);
        str = str->prepend("condition met: ");
        thread_set_event_description(*thread, str);
    }
    evalResult_.reset();
    return c;
}


////////////////////////////////////////////////////////////////
SwitchableAction::EvalEvents::EvalEvents
(
    Thread*             thread,
    BreakPoint*         bpnt,
    SwitchableAction*   action,
    Variant*            result
)
  : thread_(thread)
  , bpnt_(bpnt)
  , action_(action)
  , result_(result)
  , callRetAddr_(0)
{
    assert(result_);
}


////////////////////////////////////////////////////////////////
bool
SwitchableAction::EvalEvents::on_done(Variant* result,
                                      bool*,
                                      DebugSymbolEvents*)
{
    assert(result_);
    result_->copy(result, true);

    detach(); // detach from all observers

    if (action_->pending())
    {
        action_->execute_(thread_.get(), bpnt_.get());
    }
    return false; // do not enter interactive mode
}


////////////////////////////////////////////////////////////////
// If an error occurred while evaluating the conditional expression,
// go ahead and execute the action.
void SwitchableAction::EvalEvents::on_error(const char* error)
{
    thread_set_event_description(*thread_, error);

    Debugger* debug = CHKPTR(thread_->debugger());

    action_->execute_impl(*debug, thread_.get(), bpnt_.get());
}


////////////////////////////////////////////////////////////////
// A function in the expression that we where trying to
// evaluate caused an event -- a SEGV, an exception, or
// another breakpoint was hit.
bool
SwitchableAction::EvalEvents::on_event(Thread* thread, addr_t addr)
{
    bool handled = false;

    if ((thread_.get() == thread) && action_->pending())
    {
        // attempt to restore state
        if (CHKPTR(thread)->signal() != SIGTRAP)
        {
            thread->set_signal(0);
            interface_cast<Runnable&>(*thread).set_program_count(callRetAddr_);
        }
        handled = true;
    }
    return handled;
}


////////////////////////////////////////////////////////////////
void SwitchableAction::EvalEvents::on_call(addr_t addr, Symbol*)
{
    callRetAddr_ = addr;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
