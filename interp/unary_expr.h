#ifndef UNARY_EXPR_H__A51C0872_D9A1_4361_8DC3_C5FFE4FD49FE
#define UNARY_EXPR_H__A51C0872_D9A1_4361_8DC3_C5FFE4FD49FE
//
// $Id: unary_expr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "expr.h"

/**
 * Models C/C++ unary expressions
 */
CLASS UnaryExpr : public Expr
{
BEGIN_INTERFACE_MAP(UnaryExpr)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

public:
    enum Operator
    {
        ADDR    = '&',
        DEREF   = '*',
        PLUS    = '+',
        MINUS   = '-',
        BITNEG  = '~',
        NEGATE  = '!',
        INCREMENT,
        DECREMENT,
        SIZE_OF,
        REFERENCE, // for internal use only
    };

    enum Source { USER, INTERNAL };

    UnaryExpr(Interp*, Operator, RefPtr<Expr>, Source = USER);

    virtual ~UnaryExpr() throw();

protected:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const;

private:
    RefPtr<Variant> eval_addr(Context&, const Variant&);

    RefPtr<Variant> eval_deref(Context&, const Variant&);

    // RefPtr<Variant> eval_plus(Context&, const Variant&);

    RefPtr<Variant> eval_minus(Context&, const Variant&);

    RefPtr<Variant> eval_bit_complement(Context&, const Variant&);

    RefPtr<Variant> eval_negate(Context&, const Variant&);

    RefPtr<Variant> eval_sizeof(Context&);

    bool try_overloaded_operator(Context&, RefPtr<Variant>);

private:
    Operator oper_;
    RefPtr<Expr> arg_;
    Source src_; // USER or INTERNAL
};

#endif // UNARY_EXPR_H__A51C0872_D9A1_4361_8DC3_C5FFE4FD49FE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
