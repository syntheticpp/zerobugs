// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: relational_expr.cpp 714 2010-10-17 10:03:52Z root $
//
#include "zdk/types.h"
#include "typez/public/type_tags.h"
#include "context.h"
#include "relational_expr.h"
#include "variant_impl.h"
#include <stdexcept>
#include <string>



RelationalExpr::RelationalExpr
(
    Interp*         interp,
    Operator        oper,
    RefPtr<Expr>    lhs,
    RefPtr<Expr>    rhs
)
: BinaryExpr(interp, lhs, rhs), oper_(oper)
{
}


RefPtr<Variant> RelationalExpr::eval_impl(Context& context)
{
    RefPtr<Variant> lval, rval;
    eval_operands(context, lval, rval);

    int result = 0;

    if (lval->type_tag() == Variant::VT_POINTER)
    {
        if (rval->type_tag() == Variant::VT_POINTER)
        {
            // compare pointers

            // TODO: ensure that pointers are of same type?
            // TODO: make sure they are not of type void?
            if (lval->pointer() < rval->pointer())
            {
                result = -1;
            }
            else if (lval->pointer() > rval->pointer())
            {
                result = 1;
            }
        }
        else
        {
            // can't compare pointers to non-pointers
            throw_invalid_types();
        }
    }
    else if (is_integer(*lval))
    {
        if (rval->type_tag() == Variant::VT_POINTER)
        {
            // can't compare integers to pointers
            throw_invalid_types();
        }
        else if (is_integer(*rval))
        {
            result = compare_integers(context, *lval, *rval);
        }
        else if (is_float(*rval))
        {
            result = compare_int_to_float(*lval, *rval);
        }
    }
    else if (is_float(*lval))
    {
        if (rval->type_tag() == Variant::VT_POINTER)
        {
            // can't compare floats to pointers
            throw_invalid_types();
        }
        else if (is_integer(*rval))
        {
            // NOTE: operands are transposed, hence the negation
            result = -compare_int_to_float(*rval, *lval);
        }
        else if (is_float(*rval))
        {
            result = compare_floats(*lval, *rval);
        }
    }

    switch (oper_)
    {
    case EQ:    result = (result == 0);   break;
    case NEQ:   result = (result != 0);   break;
    case LT:    result = (result < 0);    break;
    case LTE:   result = (result <= 0);   break;
    case GT:    result = (result > 0);    break;
    case GTE:   result = (result >=0);    break;

    default: assert(false);
    }
    assert(result >= 0);

    // the type of the resulting expression is int
    // Q: should I go C++ style and make the type bool?
    RefPtr<DataType> type = context.get_int_type();
    set_type(type);

    RefPtr<Variant> var = new VariantImpl;
    //variant_assign(*var, *type, result);
    put(var.get(), result);
    return var;
}


int RelationalExpr::compare_int_to_float(
    const Variant& lhs,
    const Variant& rhs)
{
    assert(is_integer(lhs));
    assert(is_float(rhs));

    if (lhs.type_tag() == Variant::VT_UINT64)
    {
        const uint64_t lval = lhs.uint64();

        if (lval < rhs.long_double())
        {
            return -1;
        }
        return lval > rhs.long_double();
    }
    else
    {
        const int64_t lval = lhs.int64();

        if (lval < rhs.long_double())
        {
            return -1;
        }
        return lval > rhs.long_double();
    }
    assert(false); // should never get here
    return 0;
}


int RelationalExpr::compare_integers(Context& context,
                                     const Variant& lhs,
                                     const Variant& rhs)
{
    assert(is_integer(lhs));
    assert(is_integer(rhs));

    if (is_signed(lhs) && !is_signed(rhs))
    {
        context.notify_warning_event(
            "Warning: comparing signed to unsigned");
    }
    else if (!is_signed(lhs) && is_signed(rhs))
    {
        context.notify_warning_event(
            "Warning: comparing unsigned to signed");
    }

    if (lhs.type_tag() == Variant::VT_UINT64)
    {
        uint64_t lval = lhs.uint64();

        if (rhs.type_tag() == Variant::VT_UINT64)
        {
            if (lval < rhs.uint64())
            {
                return -1;
            }
            else
            {
                return lval > rhs.uint64();
            }
        }
        else
        {
            if (lval < rhs.int64())
            {
                return -1;
            }
            else
            {
                return lval > rhs.int64();
            }
        }
    }
    else
    {
        int64_t lval = lhs.int64();

        if (rhs.type_tag() == Variant::VT_UINT64)
        {
            if (lval < rhs.uint64())
            {
                return -1;
            }
            else
            {
                return lval > rhs.uint64();
            }
        }
        else
        {
            if (lval < rhs.int64())
            {
                return -1;
            }
            else
            {
                return lval > rhs.int64();
            }
        }
    }
    return 0;
}


int RelationalExpr::compare_floats(
    const Variant& lhs,
    const Variant& rhs)
{
    assert(is_float(lhs));
    assert(is_float(rhs));

    long double lval = lhs.long_double();
    long double rval = rhs.long_double();

    if (lval < rval)
    {
        return -1;
    }
    return lval > rval;
}


const char* RelationalExpr::operator_name() const
{
    switch (oper_)
    {
        case EQ: return "==";
        case NEQ: return "!=";
        case LT: return "<";
        case GT: return ">";
        case LTE: return "<=";
        case GTE: return ">=";
    }
    assert(false);
    return "";
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
