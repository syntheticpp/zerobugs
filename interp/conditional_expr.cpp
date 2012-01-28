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

#include "conditional_expr.h"
#include "debug_out.h"
#include "errors.h"
#include "interp.h"

using namespace std;


ConditionalExpr::ConditionalExpr
(
    Interp* interp,
    RefPtr<Expr> discr,
    RefPtr<Expr> first,
    RefPtr<Expr> second
)
  : Expr(interp)
  , discr_(discr)
  , first_(first)
  , second_(second)
{
    assert(discr_.get());
    assert(first_.get());
    assert(second_.get());
}


RefPtr<Variant> ConditionalExpr::eval_impl(Context& context)
{
    RefPtr<Variant> var = discr_->eval(context);
    // post-conditions:
    if (!discr_->type())
    {
        throw logic_error("null type in conditional expression");
    }
    if (!var)
    {
        throw logic_error("error evaluating conditional expression");
    }
    bool value = 0;
    if (is_integer(*var))
    {
        value = (var->bits() != 0);
    }
    else if (is_float(*var))
    {
        value = (var->long_double() != 0);
    }
    else if (var->type_tag() == Variant::VT_POINTER)
    {
        value = (var->pointer() != 0);
    }
    else
    {
        throw EvalError("invalid type in conditional expression");
    }
    if (value)
    {
        var = first_->eval(context);
        DEBUG_OUT << "conditional, first=" << first_->type()->name() << endl;
        set_type(first_->type());
    }
    else
    {
        var = second_->eval(context);
        DEBUG_OUT << "conditional, second=" << second_->type()->name() << endl;
        set_type(second_->type());
    }
    return var;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
