#ifndef MULTIPLICATIVE_EXPR_H__61EB9A8A_AB9A_4C96_BEBF_3BE0AA97770E
#define MULTIPLICATIVE_EXPR_H__61EB9A8A_AB9A_4C96_BEBF_3BE0AA97770E
//
// $Id: multiplicative_expr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "binary_expr.h"


CLASS MultiplicativeExpr : public BinaryExpr
{
public:
    enum Operator { MUL = '*', DIV = '/', MOD = '%' };

    MultiplicativeExpr(
        Interp*, Operator, RefPtr<Expr> lhs, RefPtr<Expr> rhs);

protected:
    RefPtr<Variant> eval_impl(Context&);

    const char* operator_name() const;

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new MultiplicativeExpr(interp, oper_,
                                      lhs()->clone(interp),
                                      rhs()->clone(interp));
    }

private:
    // called when both lhs and rhs are ints
    void eval_int_by_int(const Variant&, const Variant&, Variant& result);

    // one operand is int, the other is float
    void eval_int_by_float(const Variant&, const Variant&, Variant&);
    void eval_float_by_int(const Variant&, const Variant&, Variant&);

    // both operands are float
    void eval_float_by_float(const Variant&, const Variant&, Variant&);

    Operator oper_;
};

// just a shorthand...
typedef MultiplicativeExpr MulExpr;

#endif // MULTIPLICATIVE_EXPR_H__61EB9A8A_AB9A_4C96_BEBF_3BE0AA97770E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
