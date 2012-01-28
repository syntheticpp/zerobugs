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

#include "context.h"
#include "logical_expr.h"
#include "variant_convert.h"
#include "typez/public/type_tags.h" // put<>
#include "variant_impl.h"


LogicalExpr::LogicalExpr (
    Interp*         interp,
    Operator        oper,
    RefPtr<Expr>    lhs,
    RefPtr<Expr>    rhs
    )
  : BinaryExpr(interp, lhs, rhs), oper_(oper)
{
}


RefPtr<Variant> LogicalExpr::eval_impl(Context& context)
{
    RefPtr<Variant> lval, rval, result;
    int bits = 0;

    RefPtr<DataType> intType = context.get_int_type();
    // resulting type is int for compatibility with C
    set_type(intType);

    eval_operand(context, lval, true);
    variant_convert(lval, *intType); // ensure integer type

    bits = lval->bits();
    if ((bits && (oper_ == OR))
        || ((bits == 0) && (oper_ == AND))
       )
    {
        result = new VariantImpl;
        put(result.get(), bits);

        return result;
    }

    eval_operand(context, rval, false);
    variant_convert(rval, *intType);

    bits = lval->bits();

    if (oper_ == AND)
    {
        bits = bits && rval->bits();
    }
    else
    {
        bits = bits || rval->bits();
    }
    result = new VariantImpl;
    put(result.get(), bits);

    return result;
}


const char* LogicalExpr::operator_name() const
{
    switch (oper_)
    {
    case AND: return "&&";
    case OR:  return "||";
    }
    assert(false);
    return "";
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
