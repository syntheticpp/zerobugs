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

#include "assign.h"
#include "context.h"
#include "errors.h"
#include "variant_convert.h"
#include "variant_impl.h"


Assign::Assign(Interp* interp, RefPtr<Expr> lhs, RefPtr<Expr> rhs)
    : Expr(interp), lhs_(lhs), rhs_(rhs)
{
    if (!lhs_)
    {
        throw std::logic_error("null left operand in assignment");
    }
    if (!rhs_)
    {
        throw std::logic_error("null right operand in assignment");
    }
}


Assign::~Assign() throw ()
{
}


RefPtr<Variant> Assign::eval_impl(Context& context)
{
    RefPtr<Variant> lval = lhs_->eval(context);
    RefPtr<Variant> rval = rhs_->eval(context);

    if (!lval)
    {
        throw std::logic_error("null lvalue in assignment");
    }
    if (!rval)
    {
        throw std::logic_error("null right hand-side in assignment");
    }
    RefPtr<DebugSymbol> symbol = lval->debug_symbol();

    if (!symbol || symbol->is_constant() || symbol->is_return_value())
    {
        throw EvalError("non-lvalue in assignment");
    }
    if (!symbol->type())
    {
        throw std::logic_error("null symbol type");
    }

    if (lval->type_tag() == Variant::VT_POINTER)
    {
        if (rval->type_tag() != Variant::VT_POINTER)
        {
            assert(rhs_->type().get() && rhs_->type()->name());

            std::string err = "cannot convert ";
            err += rhs_->type()->name()->c_str();
            err += " to pointer";

            throw EvalError(err);
        }
    }
    else if (lval->type_tag() == Variant::VT_ARRAY)
    {
        throw EvalError("assignment to array not supported");
    }
    else if (lval->type_tag() == Variant::VT_OBJECT)
    {
        throw EvalError("assignment to object not supported");
    }
    else if (lval->type_tag() != rval->type_tag())
    {
        variant_convert(rval, *symbol->type());
    }
    assert(rval->size() == lval->size());
    assert(rval->type_tag() == lval->type_tag());

    set_type(lhs_->type());

    context.commit_value(*symbol, *rval);

    return new VariantImpl(*symbol);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
