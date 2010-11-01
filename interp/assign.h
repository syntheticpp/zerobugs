#ifndef ASSIGN_H__5DAA968F_F839_4428_8039_51181314EDEA
#define ASSIGN_H__5DAA968F_F839_4428_8039_51181314EDEA
//
//
// $Id: assign.h 714 2010-10-17 10:03:52Z root $
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


CLASS Assign : public Expr
{
BEGIN_INTERFACE_MAP(AssignmentExpr)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

public:
    Assign(Interp*, RefPtr<Expr>, RefPtr<Expr>);

    virtual ~Assign() throw();

protected:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* in) const
    {
        return new Assign(in, lhs_->clone(in), rhs_->clone(in));
    }

private:
    RefPtr<Expr> lhs_;
    RefPtr<Expr> rhs_;
};
#endif // ASSIGNMENT_H__5DAA968F_F839_4428_8039_51181314EDEA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
