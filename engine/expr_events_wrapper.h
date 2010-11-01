#ifndef EXPR_EVENTS_WRAPPER_H__F3F1617F_3193_4B35_B1BD_755D1CE20567
#define EXPR_EVENTS_WRAPPER_H__F3F1617F_3193_4B35_B1BD_755D1CE20567
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: expr_events_wrapper.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/expr.h"
#include "zdk/observer_impl.h"


using Platform::addr_t;


template<typename T>
class ExprEventsWrapper : public SubjectImpl<ExprEvents>
{
    RefPtr<ExprEvents> events_; // wrapped events object

protected:
    ExprEventsWrapper(const ExprEventsWrapper& other)
        : events_(other.events_)
    { }

public:
    explicit ExprEventsWrapper(ExprEvents* events) : events_(events)
    { }

    bool on_done(Variant* var, bool* interactive, DebugSymbolEvents* dse = 0)
    {
        if (events_)
        {
            return events_->on_done(var, interactive, dse);
        }
        return interactive ? *interactive : false;
    }

    void on_error(const char* msg)
    {
        if (events_) events_->on_error(msg);
    }

    void on_warning(const char* msg)
    {
        if (events_) events_->on_warning(msg);
    }

    bool on_event(Thread* thread, addr_t addr)
    {
        return events_ ? events_->on_event(thread, addr) : false;
    }

    void on_call(addr_t addr, Symbol* sym = 0)
    {
        if (events_) events_->on_call(addr, sym);
    }

    ExprEvents* clone() const { return new T(static_cast<const T&>(*this)); }
};

#endif // EXPR_EVENTS_WRAPPER_H__F3F1617F_3193_4B35_B1BD_755D1CE20567
