#ifndef EXPR_OBSERVER_H__EDCAB31C_927B_4D7A_8B30_97518B774B7B
#define EXPR_OBSERVER_H__EDCAB31C_927B_4D7A_8B30_97518B774B7B
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

#include <vector>
#include "zdk/expr.h"
#include "zdk/observer_impl.h"


CLASS ExprObserver : public ObserverImpl<>
{
public:
    typedef std::vector<WeakPtr<ExprEvents> > Events;

    static RefPtr<ExprObserver> create()
    { return new ExprObserver; }

    void add_events(ExprEvents& e)
    {
        if (add_event(&e))
        {
            e.attach(this);
        }
    }

    const Events& events() const { return events_; }

protected:
    bool add_event(ExprEvents* e)
    {
        // remove NULLs
        std::set<WeakPtr<ExprEvents> > tmp(events_.begin(),
                                           events_.end());
        tmp.erase(WeakPtr<ExprEvents>());

        const bool result = tmp.insert(e).second;
        Events(tmp.begin(), tmp.end()).swap(events_);

        return result;
    }

    // void on_state_change(Subject* subject) { }

    ExprObserver() { }

    virtual ~ExprObserver() throw() { }

private:
    Events events_;
};

#endif // EXPR_OBSERVER_H__EDCAB31C_927B_4D7A_8B30_97518B774B7B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
