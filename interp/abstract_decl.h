#ifndef ABSTRACT_DECL_H__8ECF433B_18D5_47D2_BB31_8FA36BA0D448
#define ABSTRACT_DECL_H__8ECF433B_18D5_47D2_BB31_8FA36BA0D448
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
//
#include "zdk/types.h"
#include "expr.h"


////////////////////////////////////////////////////////////////
CLASS AbstractDeclarator : public Expr
{
public:
    DECLARE_UUID("7b7fc8c1-fe9e-4e6d-a902-4bf2bada43c8")

BEGIN_INTERFACE_MAP(AbstractDeclarator)
    INTERFACE_ENTRY(AbstractDeclarator)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    virtual ~AbstractDeclarator() throw();

    explicit AbstractDeclarator(Interp*);

    RefPtr<Expr> apply(RefPtr<Expr>);

    void set_right_node(const RefPtr<AbstractDeclarator>&);

protected:
    RefPtr<Expr> expr() const { return expr_; }

private:
    RefPtr<Expr> expr_;
    RefPtr<AbstractDeclarator> right_;
};


////////////////////////////////////////////////////////////////
CLASS RightAssocExpr : public Expr
{
public:
    DECLARE_UUID("2fb89898-d612-472d-bd00-2d7e96c36234")

BEGIN_INTERFACE_MAP(RightAssocExpr)
    INTERFACE_ENTRY(RightAssocExpr)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    virtual ~RightAssocExpr() throw() {};

    RightAssocExpr(Interp* interp, RefPtr<Expr> child)
        : Expr(interp)
        , child_(child)
    { assert(child_.get()); }

    RefPtr<Expr> child() const { return child_; }

private:
    RefPtr<Variant> eval_impl(Context& ctxt)
    { return child_->eval(ctxt); }

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new RightAssocExpr(interp, child_->clone(interp));
    }

private:
    RefPtr<Expr> child_;
};


////////////////////////////////////////////////////////////////
CLASS ArrayDeclarator : public AbstractDeclarator
{
public:
    DECLARE_UUID("db3c49ee-cb54-484a-8cdd-f52d10196e14")
BEGIN_INTERFACE_MAP(ArrayDeclarator)
    INTERFACE_ENTRY(ArrayDeclarator)
    INTERFACE_ENTRY_INHERIT(AbstractDeclarator);
END_INTERFACE_MAP()

    explicit ArrayDeclarator(Interp*);

    ArrayDeclarator(Interp*, RefPtr<Expr>);

protected:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        if (!sizeExpr_)
        {
            return new ArrayDeclarator(interp);
        }
        else
        {
            return new ArrayDeclarator(interp, sizeExpr_->clone(interp));
        }
    }

private:
    RefPtr<Expr> sizeExpr_;
};


////////////////////////////////////////////////////////////////
CLASS PointerDeclarator : public AbstractDeclarator
{
public:
    DECLARE_UUID("f01b98cf-f35b-45ba-88df-3792a1c5f006")

BEGIN_INTERFACE_MAP(PointerDeclarator)
    INTERFACE_ENTRY(PointerDeclarator)
    INTERFACE_ENTRY_INHERIT(AbstractDeclarator);
END_INTERFACE_MAP()

    explicit PointerDeclarator(Interp*);

    PointerDeclarator(Interp*, RefPtr<Expr> qualifiers);

protected:
    RefPtr<Variant> eval_impl(Context&);

    PointerDeclarator(Interp* interp, Qualifier qual)
        : AbstractDeclarator(interp), qual_(qual)
    { }

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new PointerDeclarator(interp, qual_);
    }

    Qualifier qual_;
};


////////////////////////////////////////////////////////////////
CLASS ReferenceDeclarator : public AbstractDeclarator
{
public:
    DECLARE_UUID("34b338c8-58f5-49d7-b777-606811d95de6")

BEGIN_INTERFACE_MAP(ReferenceDeclarator)
    INTERFACE_ENTRY(ReferenceDeclarator)
    INTERFACE_ENTRY_INHERIT(AbstractDeclarator);
END_INTERFACE_MAP()

    explicit ReferenceDeclarator(Interp*);

protected:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    { return new ReferenceDeclarator(interp); }
};


////////////////////////////////////////////////////////////////
CLASS FunctionDeclarator : public AbstractDeclarator
{
public:
    DECLARE_UUID("53ada4b5-a27f-46e2-a023-c46ac53ab765")

BEGIN_INTERFACE_MAP(FunctionDeclarator)
    INTERFACE_ENTRY(FunctionDeclarator)
    INTERFACE_ENTRY_INHERIT(AbstractDeclarator);
END_INTERFACE_MAP()

    explicit FunctionDeclarator(Interp*);

    FunctionDeclarator(Interp*, RefPtr<Expr> paramList);

protected:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        if (!param_)
        {
            return new FunctionDeclarator(interp);
        }
        else
        {
            return new FunctionDeclarator(interp, param_->clone(interp));
        }
    }

private:
    RefPtr<Expr> param_;
};



RefPtr<Expr> combine_decl(RefPtr<Expr>, RefPtr<Expr>);


#endif // ABSTRACT_DECL_H__8ECF433B_18D5_47D2_BB31_8FA36BA0D448
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
