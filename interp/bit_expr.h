#ifndef BITWISE_EXPR_H__440B654B_BA9B_4F0A_95C4_26AE47501767
#define BITWISE_EXPR_H__440B654B_BA9B_4F0A_95C4_26AE47501767
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

#include "binary_expr.h"


CLASS BitExpr : public BinaryExpr
{
public:
    enum Oper
    {
        LEFT_SHIFT,
        RIGHT_SHIFT,
        AND,
        OR,
        XOR,
    };

    BitExpr(Interp*, Oper, RefPtr<Expr> lhs, RefPtr<Expr> rhs);

protected:
    RefPtr<Variant> eval_impl(Context&);

    const char* operator_name() const;

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new BitExpr(interp,
                            oper_,
                            lhs()->clone(interp),
                            rhs()->clone(interp));
    }

private:
    Oper oper_;
};
#endif // BITWISE_EXPR_H__440B654B_BA9B_4F0A_95C4_26AE47501767
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
