#ifndef EVAL_BASE_H__A538607C_CBDE_4861_899C_9BFF5EFB08C0
#define EVAL_BASE_H__A538607C_CBDE_4861_899C_9BFF5EFB08C0
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: eval_base.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <string>
#include "eval_events.h"
#include "gtkmm/connect.h"


class VariablesView;



template<typename T, typename B>
class ZDK_LOCAL EvalBase : public B
{
    template<typename V>
    SignalProxy<V, EvalBase> wrap(V& s)
    {
        return SignalProxy<V, EvalBase>(s, *this);
    }

public:
    typedef EvalBase<T, B> Base;

    template<typename A0>
    explicit EvalBase(A0& a0) : B(a0)
    { }

    template<typename A0, typename A1>
    EvalBase(A0 a0, A1 a1, Thread& thread)
        : B(a0, a1)
        , events_(EvalEvents::create(&thread))
    {
        connect_signals();
    }

    // implement mutex behavior, for use with SignalProxy wrapper above
    void enter() { if (events_) events_->mutex().enter(); }
    void leave() { if (events_) events_->mutex().leave(); }

    SignalProxy<SigC::Signal4<bool, std::string, addr_t, ExprEvents*, int>, EvalBase>
    signal_evaluate() { return wrap(evaluate); }

    SignalProxy<SigC::Signal1<void, VariablesView*>, EvalBase> signal_refresh()
    { return wrap(refresh); }

    SignalProxy<SigC::Signal1<void, std::string>, EvalBase> signal_eval_error()
    { return wrap(eval_error); }

    SignalProxy<SigC::Signal1<void, std::string>, EvalBase> signal_eval_warn()
    { return wrap(eval_warn); }

    SignalProxy<SigC::Signal3<void, std::string, bool*, const char*>, EvalBase>
    signal_confirm() { return wrap(confirm); }

protected:
    SigC::Signal4<bool, std::string, addr_t, ExprEvents*, int> evaluate;
    SigC::Signal1<void, VariablesView*> refresh;
    SigC::Signal1<void, std::string> eval_error;
    SigC::Signal1<void, std::string> eval_warn;
    SigC::Signal3<void, std::string, bool*, const char*> confirm;

    /**
     * @note runs on main thread; argument passed by value on purpose
     * @return true to keep the events observer active
     */
    virtual bool on_error(std::string msg)
    {
        eval_error.emit(msg);
        return true;
    }
    /**
     * @note return value is ignored, just for compat with on_error
     * when sharing the same signal slot
     */
    virtual bool on_warn(std::string msg)
    {
        eval_warn.emit(msg);
        return true;
    }

    EvalEvents* events() { return events_.get(); }

    void connect_signals()
    {
        if (events_)
        {
            Lock<Mutex> lock(events_->mutex());
            // arrange for the EvalEvents object to feed back
            // notifications to this object
            events_->signal_error()->connect(Gtk_SLOT(this, &EvalBase::on_error));
            events_->signal_warning()->connect(Gtk_SLOT(this, &EvalBase::on_warn));

            events_->signal_confirm()->connect(confirm.slot());
            events_->signal_done()->connect(Gtk_SLOT(static_cast<T*>(this), &T::on_done));
        }
    }

    RefPtr<EvalEvents> events_;
};
#endif // EVAL_BASE_H__A538607C_CBDE_4861_899C_9BFF5EFB08C0
