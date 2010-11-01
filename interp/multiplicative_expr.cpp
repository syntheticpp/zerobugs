//
// $Id: multiplicative_expr.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <stdexcept>
#include <limits>
#include "errors.h"
#include "multiplicative_expr.h"
#include "variant_assign.h"
#include "variant_impl.h"



MultiplicativeExpr::MultiplicativeExpr
(
    Interp*         interp,
    Operator        oper,
    RefPtr<Expr>    lhs,
    RefPtr<Expr>    rhs
)
: BinaryExpr(interp, lhs, rhs), oper_(oper)
{
}


const char* MultiplicativeExpr::operator_name() const
{
    switch(oper_)
    {
    case DIV: return "/";
    case MUL: return "*";
    case MOD: return "%";
    }
    assert(false);
    return "";
}


RefPtr<Variant> MultiplicativeExpr::eval_impl(Context& context)
{
    RefPtr<Variant> lval, rval;
    eval_operands(context, lval, rval);

    RefPtr<Variant> result = new VariantImpl;

    if (lval->type_tag() == Variant::VT_POINTER)
    {
        throw_invalid_types();
    }
    else if (lval->type_tag() == Variant::VT_OBJECT)
    {
        try_overloaded_operator(context);
    }
    else if (rval->type_tag() == Variant::VT_OBJECT)
    {
        try_standalone_operator(context);
    }
    else if (is_integer(*lval))
    {
        if (rval->type_tag() == Variant::VT_POINTER)
        {
            throw_invalid_types();
        }
        else if (is_integer(*rval))
        {
            eval_int_by_int(*lval, *rval, *result);
        }
        else if (is_float(*rval))
        {
            if (oper_ == MOD)
            {
                throw_invalid_types();
            }
            eval_int_by_float(*lval, *rval, *result);
        }
    }
    else if (is_float(*lval))
    {
        if (oper_ == MOD)
        {
            throw_invalid_types();
        }
        if (rval->type_tag() == Variant::VT_POINTER)
        {
            throw_invalid_types();
        }
        else if (is_integer(*rval))
        {
            eval_float_by_int(*lval, *rval, *result);
        }
        else if (is_float(*rval))
        {
            eval_float_by_float(*lval, *rval, *result);
        }
    }
    return result;
}


void MultiplicativeExpr::eval_int_by_int(
    const Variant&  lval,
    const Variant&  rval,
    Variant&        result)
{
    if (lval.type_tag() == Variant::VT_UINT64)
    {
        uint64_t v = lval.uint64();

        // Set the resulting type to the LHS type
        //
        // The Rule: if at least one of the operands is unsigned 64-bit,
        // set the result type to uint64_t.

        set_type(lhs()->type());

        if (rval.type_tag() == Variant::VT_UINT64)
        {
            switch (oper_)
            {
            case MUL: v *= rval.uint64();
                break;
            case MOD:
                if (uint64_t d = rval.uint64())
                {
                    v %= d;
                }
                else throw EvalError("division by zero");
                break;
            case DIV:
                if (uint64_t d = rval.uint64())
                {
                    v /= d;
                }
                else throw EvalError("division by zero");
                break;
            }
            variant_assign(result, *type(), v);
        }
        else
        {
            switch (oper_)
            {
            case MUL: v *= rval.int64();
                break;
            case MOD:
                if (int64_t d = rval.int64())
                {
                    v %= d;
                }
                else throw EvalError("division by zero");
            case DIV:
                if (int64_t d = rval.int64())
                {
                    v /= d;
                }
                else throw EvalError("division by zero");
                break;
            }
            variant_assign(result, *type(), v);
        }
    }
    else
    {
        int64_t v = lval.int64();

        if (rval.type_tag() == Variant::VT_UINT64)
        {
            // if at least one of the operands is unsigned 64-bit,
            // set the result type to uint64_t
            set_type(rhs()->type());

            switch (oper_)
            {
            case MUL: v *= rval.uint64(); break;
            case MOD: v %= rval.uint64(); break;

            case DIV:
                if (uint64_t d = rval.uint64())
                {
                    v /= d;
                }
                else throw EvalError("division by zero");
                break;
            }
            variant_assign(result, *type(), v);
        }
        else
        {
            // none of the operands is 64-bit, prefer
            // the larger unsigned type
            set_int_type(lval, rval);
            assert(type().get());

            switch (oper_)
            {
            case MUL: v *= rval.int64(); break;
            case MOD: v %= rval.int64(); break;

            case DIV:
                if (uint64_t d = rval.int64())
                {
                    v /= d;
                }
                else throw EvalError("division by zero");
                break;
            }
            variant_assign(result, *type(), v);
        }
    }
    // postconditions
    assert(result.type_tag() != Variant::VT_NONE);
    assert(type().get());
}


void MultiplicativeExpr::eval_int_by_float(
    const Variant&  lval,
    const Variant&  rval,
    Variant&        result)
{
    assert(is_integer(lval));
    assert(is_float(rval));

    set_type(rhs()->type());

    long double v = rval.long_double();

    if (lval.type_tag() == Variant::VT_UINT64)
    {
        switch (oper_)
        {
        case MOD: assert(false); break;

        case MUL: v *= lval.uint64(); break;
        case DIV:
            // greater than zero, with tolerance?
            if (v > std::numeric_limits<long double>::epsilon())
            {
                v = lval.uint64() / v;
                break;
            }
            throw EvalError("division by zero");
        }
    }
    else
    {
        switch (oper_)
        {
        case MOD: assert(false); break;
        case MUL: v *= lval.int64(); break;
        case DIV:
            // greater than zero, with tolerance?
            if (v > std::numeric_limits<long double>::epsilon())
            {
                v = lval.int64() / v;
                break;
            }
            throw EvalError("division by zero");
        }
    }
    variant_assign(result, *type(), v);
}


void MultiplicativeExpr::eval_float_by_int(
    const Variant&  lval,
    const Variant&  rval,
    Variant&        result)
{
    assert(is_float(lval));
    assert(is_integer(rval));

    set_type(lhs()->type());

    long double v = lval.long_double();

    if (rval.type_tag() == Variant::VT_UINT64)
    {
        switch (oper_)
        {
        case MOD: assert(false); break;
        case MUL: v *= rval.uint64(); break;
        case DIV:
            if (uint64_t d = rval.uint64())
            {
                v /= d;
            }
            else throw EvalError("division by zero");
            break;
        }
    }
    else
    {
        switch (oper_)
        {
        case MOD: assert(false); break;
        case MUL: v *= rval.int64(); break;
        case DIV:
            if (int64_t d = rval.int64())
            {
                v /= d;
            }
            else throw EvalError("division by zero");
            break;
        }
    }
    variant_assign(result, *type(), v);
}


void MultiplicativeExpr::eval_float_by_float(
    const Variant&  lval,
    const Variant&  rval,
    Variant&        result)
{
    assert(oper_ != MOD); // this should've been ruled out above

    long double v = lval.long_double();

    if (oper_ == MUL)
    {
        v *= rval.long_double();
    }
    else if (oper_ == DIV)
    {
        long double d = rval.long_double();
        if (d)
        {
            v /= d;
        }
        else
        {
            throw EvalError("division by zero");
        }
    }

    set_type(lhs()->type());
    variant_assign(result, *type(), v);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
