#ifndef ALT_EVENTS_H__62D54395_D940_4CC3_A280_CB26B86BF0AC
#define ALT_EVENTS_H__62D54395_D940_4CC3_A280_CB26B86BF0AC
//
// $Id: alt_events.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include "zdk/expr.h"
#include "zdk/observer_impl.h"
#include "zdk/symbol.h"
#include "interp.h"
#include "debug_out.h"

class Expr;

using namespace std;

/**
 * Events observer for running an alternative
 * interpreter instance.
 */
struct ZDK_LOCAL
AltEvents : public SubjectImpl<ExprEvents>
{
    RefPtr<Expr> expr_;
    RefPtr<Interp> interp_;

    AltEvents() { }

    bool on_done(Variant* result, bool* interactive, DebugSymbolEvents*)
    {
        if (interp_.get())
        {
            DEBUG_OUT << "restoring interpretor\n";
            interp_->switch_to(NULL);
        }
        if (!expr_)
        {
            return false;
        }
        if (interp_.get())
        {
            switch (interp_->resume(0, 0, interactive))
            {
            case Interp::EVAL_AGAIN:
                return false;

            case Interp::EVAL_ERROR:
                //DEBUG_OUT << "error\n";
                break;

            case Interp::EVAL_DONE:
                break;
            }
        }
        return interactive ? *interactive : false;
    }

    void on_error(const char* msg) { }

    void on_warning(const char*) { }

    bool on_event(Thread*, addr_t) { return true; }

    void on_call(addr_t addr, Symbol* sym)
    {
        if (sym)
        {
            DEBUG_OUT << "calling " << sym->name() << endl;
        }
        else
        {
            DEBUG_OUT << "returned to " << hex << addr << dec << endl;
        }
    }

protected:
    AltEvents(const AltEvents& other)
        : expr_(other.expr_)
        , interp_(other.interp_)
    { }
};
#endif // ALT_EVENTS_H__62D54395_D940_4CC3_A280_CB26B86BF0AC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
