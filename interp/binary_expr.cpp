// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: binary_expr.cpp 714 2010-10-17 10:03:52Z root $
//
#include <assert.h>
#include <sstream>
#include "zdk/check_ptr.h"
#include "zdk/types.h"
#include "binary_expr.h"
#include "call_setup.h"
#include "context.h"
#include "ident.h"
#include "interp.h"
#include "errors.h"
#include "lookup_methods.h"
#include "postfix_expr.h"
#include "unary_expr.h"
#include "variant_impl.h"

using namespace std;


BinaryExpr::BinaryExpr(Interp* interp, RefPtr<Expr> lhs, RefPtr<Expr> rhs)
    : Expr(interp), lhs_(lhs), rhs_(rhs)
{
    if (!lhs_)
    {
        throw logic_error("null left hand-side operand");
    }
    if (!rhs_)
    {
        throw logic_error("null right hand-side operand");
    }
}


BinaryExpr::~BinaryExpr() throw()
{
}


RefPtr<Expr> BinaryExpr::lhs() const
{
    assert(lhs_.get());
    return lhs_;
}


RefPtr<Expr> BinaryExpr::rhs() const
{
    assert(rhs_.get());
    return rhs_;
}


void BinaryExpr::throw_invalid_types()
{
    ostringstream os;

    os << "invalid operands of types ";
    os << lhs()->type()->name() << " and ";
    os << rhs()->type()->name() << " to operator ";
    os << operator_name();

    throw EvalError(os.str());
}



void
BinaryExpr::eval_operand(Context& context,
                         RefPtr<Variant>& var,
                         bool isLHS)
{
    RefPtr<Expr> expr = lhs();
    const char* name = "left";

    if (!isLHS)
    {
        expr = rhs();
        name = "right";
    }
    var = expr->eval(context);
    if (!var)
    {
        ostringstream msg;
        msg << "evaluating " << name << " hand-side yielded null";

        throw logic_error(msg.str());
    }
    if ((var->type_tag() == Variant::VT_NONE) || !expr->type())
    {
        ostringstream msg;
        msg << "unknown " << name << " hand-side type";

        throw logic_error(msg.str());
    }
    // arrays and object instances cannot participate in additions,
    // multiplications and logical operations
    if (var->type_tag() == Variant::VT_ARRAY)
    {
        throw_invalid_types();
    }
}


void BinaryExpr::eval_operands( Context& context,
                                RefPtr<Variant>& lval,
                                RefPtr<Variant>& rval)
{
    eval_operand(context, lval, true);
    eval_operand(context, rval, false);
}


void BinaryExpr::set_int_type(const Variant& lval, const Variant& rval)
{
    assert(is_integer(lval));
    assert(is_integer(rval));

    // set the resulting type to the largest type
    // todo: is this correct or should the type be int32?
    if (lval.size() > rval.size())
    {
        set_type(lhs()->type());
    }
    else if (lval.size() < rval.size())
    {
        set_type(rhs()->type());
    }
    else if (is_signed_int(lval))
    {
        // Types are of the same size. For consistency
        // with the rule "if one out of the two operands
        // is unsigned, the type of the result is unsigned"
        // that I applied to 64-bit operands, chose the unsigned
        // over the signed type as the result type
        set_type(rhs()->type());
    }
    else
    {
        set_type(lhs()->type());
    }
    assert(type().get());
}


void
BinaryExpr::try_standalone_operator(Context& ctxt, const char* opname)
{
    RefPtr<DebugSymbol> fun(ctxt.lookup_debug_symbol(opname));

    if (!fun)
    {
        assert(lhs()->type().get());
        string typeName = CHKPTR(lhs()->type()->name())->c_str();
        throw EvalError(opname + (" not found in: " + typeName));
    }
    else
    {
        RefPtr<Expr> lhsArg = lhs();
        if (interface_cast<ClassType>(lhs()->type()).get())
        {
            // convert to reference
            lhsArg = new UnaryExpr( this->interp().get(),
                                    UnaryExpr::REFERENCE,
                                    this->lhs(),
                                    UnaryExpr::INTERNAL);
        }
        RefPtr<Expr> rhsArg = rhs();
        if (interface_cast<ClassType>(rhs()->type()).get())
        {
            rhsArg = new UnaryExpr( this->interp().get(),
                                    UnaryExpr::REFERENCE,
                                    this->rhs(),
                                    UnaryExpr::INTERNAL);
        }
        ExprList args;
        args.push_back(lhsArg);
        args.push_back(rhsArg);

        // create the setup for calling the operator
        RefPtr<CallSetup> setup =
            get_call_setup(ctxt, *this, fun.get(), &args);

        if (setup)
        {
            assert(setup->reader() == fun->reader());

            ctxt.call_function(*setup);
        }
        else
        {
            no_matching_func(ctxt, *fun, &args);
        }
    }
}


void BinaryExpr::try_standalone_operator(Context& ctxt)
{
    string opname = "operator";
    opname += operator_name();

    try_standalone_operator(ctxt, opname.c_str());
}


void BinaryExpr::try_overloaded_operator(Context& ctxt)
{
    ClassType& klass = interface_cast<ClassType&>(*lhs()->type());
    string opname = "operator";
    opname += operator_name();

    RefPtr<DebugSymbol> fun = lookup_methods(ctxt, klass, opname);
    if (!fun)
    {
        try_standalone_operator(ctxt, opname.c_str());
    }
    else
    {
        ExprList args;
        const UnaryExpr::Source src = UnaryExpr::INTERNAL;
        args.push_back(new UnaryExpr(interp().get(), UnaryExpr::ADDR, lhs(), src));

        args.push_back(rhs());

        if (RefPtr<CallSetup> setup = get_call_setup(ctxt, *this, fun.get(), &args))
        {
            assert(setup->reader() == fun->reader());
            ctxt.call_function(*setup);
        }
        else
        {
            try_standalone_operator(ctxt, opname.c_str());
        }
    }
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
