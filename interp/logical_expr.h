#ifndef LOGICAL_EXPR_H__576B885F_6E7D_4976_93BD_C61D95B59E46
#define LOGICAL_EXPR_H__576B885F_6E7D_4976_93BD_C61D95B59E46
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


CLASS LogicalExpr : public BinaryExpr
{
public:
    enum Operator { AND, OR };

    LogicalExpr(Interp*,
                Operator,
                RefPtr<Expr> lhs,
                RefPtr<Expr> rhs);

protected:
    RefPtr<Variant> eval_impl(Context&);

    const char* operator_name() const;

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new LogicalExpr( interp,
                                oper_,
                                lhs()->clone(interp),
                                rhs()->clone(interp));
    }

private:
    Operator oper_;
};

#endif // LOGICAL_EXPR_H__576B885F_6E7D_4976_93BD_C61D95B59E46
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
