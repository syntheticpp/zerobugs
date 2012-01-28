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

#include <sstream>
#include "zdk/check_ptr.h"
#include "zdk/type_system_util.h"
#include "typez/public/param_types.h"
#include "context.h"
#include "errors.h"
#include "interp.h"
#include "parameter_list.h"
#include "qualifier_list.h"
#include "abstract_decl.h"

using namespace std;


AbstractDeclarator::~AbstractDeclarator() throw()
{
}


AbstractDeclarator::AbstractDeclarator(Interp* interp) : Expr(interp)
{
}


void AbstractDeclarator::set_right_node(
    const RefPtr<AbstractDeclarator>& decl)
{
    assert(decl.get());
    assert(decl.get() != this);

    if (!right_)
    {
        right_ = decl;
    }
    else
    {
        right_->set_right_node(decl);
    }
}


RefPtr<Expr> AbstractDeclarator::apply(RefPtr<Expr> expr)
{
    assert(expr.get() != this);
    assert(!expr_);

    expr_ = expr;

    RefPtr<AbstractDeclarator> right = right_;

    if (!right)
    {
        expr = this;
    }
    else
    {
        right_.reset();
        expr = right->apply(this);
    }
    return expr;
}


ArrayDeclarator::ArrayDeclarator(Interp* interp)
    : AbstractDeclarator(interp)
{
}


ArrayDeclarator::ArrayDeclarator(Interp* interp, RefPtr<Expr> sizeExpr)
    : AbstractDeclarator(interp)
    , sizeExpr_(sizeExpr)
{
}


RefPtr<Variant> ArrayDeclarator::eval_impl(Context& context)
{
    if (!expr())
    {
        throw logic_error("null expression in array declarator");
    }

    expr()->eval(context);
    if (expr()->type() == NULL)
    {
        throw logic_error("null type in array declarator");
    }
    if (!sizeExpr_)
    {
        throw logic_error("null size expression in array declarator");
    }
    RefPtr<Variant> size = sizeExpr_->eval(context);
    if (sizeExpr_->type() == NULL)
    {
        throw logic_error("null size type in array declarator");
    }
    if (!size)
    {
        throw logic_error("null size in array declarator");
    }
    if (!is_integer(*size))
    {
        throw EvalError("array of non-integer size type");
    }
/*
    if (size->debug_symbol() && !size->debug_symbol()->is_constant())
    {
        throw EvalError("non-constant array size type");
    }
 */
    const int64_t upper = size->type_tag() == Variant::VT_UINT64
        ? size->uint64() : size->int64();
    if (upper <= 0)
    {
        throw EvalError("array of zero or negative size");
    }
    TypeSystem& typeSys = context.type_system();
    RefPtr<DataType> elemType = CHKPTR(expr()->type());
    RefPtr<DataType> type = typeSys.get_array_type(0, upper - 1, elemType.get());
    set_type(type);

    return RefPtr<Variant>();
}


FunctionDeclarator::FunctionDeclarator(Interp* interp)
    : AbstractDeclarator(interp)
{
}


FunctionDeclarator::FunctionDeclarator(Interp* interp, RefPtr<Expr> param)
    : AbstractDeclarator(interp)
    , param_(param)
{
    assert(param_.get());
}


RefPtr<Variant> FunctionDeclarator::eval_impl(Context& context)
{
    if (!expr())
    {
        throw logic_error("null expression in function declarator");
    }

    expr()->eval(context);
    RefPtr<DataType> retType = expr()->type();

    if (!retType)
    {
        throw logic_error("null type in function declarator");
    }
    if (interface_cast<FunType*>(retType.get()))
    {
        throw EvalError("function returning function");
    }
    RefPtr<DataType> type;

    if (!param_)
    {
        type = context.type_system().get_fun_type(retType.get(), 0, 0, false);
    }
    else
    {
        ParamTypes paramTypes;
        ParamDeclList& param = interface_cast<ParamDeclList&>(*param_);

        const ExprList& list = param.list();

        ExprList::const_iterator i = list.begin();
        for (size_t n = 0; i != list.end(); ++i, ++n)
        {
            assert(i->get());
            (*i)->eval(context);

            if ((*i)->type())
            {
                WeakDataTypePtr tmp((*i)->type());
                paramTypes.push_back(tmp);
            }
            else
            {
                ostringstream err;

                err << "parameter " << n << " has null type";
                throw logic_error(err.str());
            }
        }
        TypeSystem& types = context.type_system();
        type = get_function_type(types, retType, paramTypes, false);
    }

    assert(type.get());
    set_type(type);

    return RefPtr<Variant>();
}


PointerDeclarator::PointerDeclarator(Interp* interp)
    : AbstractDeclarator(interp)
    , qual_(QUALIFIER_NONE)
{
}


PointerDeclarator::PointerDeclarator(Interp* interp, RefPtr<Expr> expr)
    : AbstractDeclarator(interp)
    , qual_(interface_cast<QualifierList&>(*expr).get())
{
}


RefPtr<Variant> PointerDeclarator::eval_impl(Context& context)
{
    assert(expr());
    if (!expr())
    {
        throw logic_error("null expression in pointer declarator");
    }

    expr()->eval(context);

    if (expr()->type() == NULL)
    {
        throw logic_error("null type in pointer declarator");
    }
    DataType* exprType = expr()->type().get();
    RefPtr<DataType> type = context.type_system().get_pointer_type(exprType);
    assert(type.get());

    type = context.type_system().get_qualified_type(type.get(), qual_);
    assert(type.get());

    set_type(type);
    return RefPtr<Variant>();
}


ReferenceDeclarator::ReferenceDeclarator(Interp* interp)
    : AbstractDeclarator(interp)
{
}


RefPtr<Variant> ReferenceDeclarator::eval_impl(Context& context)
{
    assert(expr().get());
    expr()->eval(context);
    RefPtr<DataType> exprType = expr()->type();
    if (!exprType)
    {
        throw logic_error("null type in reference declarator");
    }
    RefPtr<DataType> type =
        context.type_system().get_reference_type(exprType.get());

    assert(type.get());
    set_type(type);

    return RefPtr<Variant>();
}


RefPtr<Expr> combine_decl(RefPtr<Expr> lhs, RefPtr<Expr> rhs)
{
    if (!lhs)
    {
        throw logic_error("null left hand-side declarator");
    }
    if (!rhs)
    {
        throw logic_error("null right hand-side declarator");
    }
    if (RightAssocExpr* expr = interface_cast<RightAssocExpr*>(lhs.get()))
    {
        RefPtr<AbstractDeclarator> decl =
            &interface_cast<AbstractDeclarator&>(*expr->child());

        interface_cast<AbstractDeclarator&>(*rhs).set_right_node(decl);
        return rhs;
    }
    else
    {
        interface_cast<AbstractDeclarator&>(*lhs).set_right_node(
            interface_cast<AbstractDeclarator>(rhs));
        return lhs;
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
