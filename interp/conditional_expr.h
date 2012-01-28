#ifndef CONDITIONAL_EXPR_H__557486FD_CA30_4631_AA40_8EFF80639491
#define CONDITIONAL_EXPR_H__557486FD_CA30_4631_AA40_8EFF80639491
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

#include "expr.h"

CLASS ConditionalExpr : public Expr
{
public:
    ConditionalExpr(Interp*,
                    RefPtr<Expr> discriminator,
                    RefPtr<Expr> first,
                    RefPtr<Expr> second);

    RefPtr<Variant> eval_impl(Context&);

BEGIN_INTERFACE_MAP(ConditionalExpr)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

protected:
    RefPtr<Expr> clone(Interp* interp) const
    {
        return new ConditionalExpr(interp,
                                discr_->clone(interp),
                                first_->clone(interp),
                                second_->clone(interp));
    }

private:
    RefPtr<Expr> discr_;
    RefPtr<Expr> first_;
    RefPtr<Expr> second_;
};


#endif // CONDITIONAL_EXPR_H__557486FD_CA30_4631_AA40_8EFF80639491
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
