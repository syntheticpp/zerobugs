#ifndef PRIMARY_EXPR_H__8BAD13AD_04DE_441F_8F53_2B578C28448C
#define PRIMARY_EXPR_H__8BAD13AD_04DE_441F_8F53_2B578C28448C
//
// $Id: primary_expr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "expr.h"


CLASS PrimaryExpr : public Expr
{
public:
    virtual ~PrimaryExpr() throw();

protected:
    explicit PrimaryExpr(Interp*);
};

#endif // PRIMARY_EXPR_H__8BAD13AD_04DE_441F_8F53_2B578C28448C
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
