#ifndef RELATIONAL_EXPR_H__FB8F917F_6C10_4637_85E9_5B8A99BB6DED
#define RELATIONAL_EXPR_H__FB8F917F_6C10_4637_85E9_5B8A99BB6DED
//
// $Id: relational_expr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "binary_expr.h"


CLASS RelationalExpr : public BinaryExpr
{
public:
    enum Operator
    {
        EQ,     // equals
        NEQ,    // not equal
        LT,     // less than
        GT,     // greater than
        LTE,    // less than or equals
        GTE,    // greater than or equals
    };

    RelationalExpr(Interp*, Operator, RefPtr<Expr> lhs, RefPtr<Expr> rhs);

BEGIN_INTERFACE_MAP(RelationalExpr)
    INTERFACE_ENTRY_INHERIT(BinaryExpr)
END_INTERFACE_MAP()

protected:
    RefPtr<Variant> eval_impl(Context&);

    const char* operator_name() const;

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new RelationalExpr(interp, oper_,
                                lhs()->clone(interp),
                                rhs()->clone(interp));
    }

private:
    int compare_int_to_float(const Variant&, const Variant&);

    int compare_integers(Context&, const Variant& lhs, const Variant& rhs);

    int compare_floats(const Variant&, const Variant&);

private:
    Operator oper_;
};
#endif // RELATIONAL_EXPR_H__FB8F917F_6C10_4637_85E9_5B8A99BB6DED
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
