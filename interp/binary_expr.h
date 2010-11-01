#ifndef BINARY_EXPR_H__3635E1B0_AE6D_4C0D_ABAF_7A9A3C06914D
#define BINARY_EXPR_H__3635E1B0_AE6D_4C0D_ABAF_7A9A3C06914D
//
// $Id: binary_expr.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/variant.h"
#include "expr.h"


CLASS BinaryExpr : public Expr
{
protected:
    BinaryExpr(Interp*, RefPtr<Expr> left, RefPtr<Expr> right);

    virtual const char* operator_name() const = 0;

    /**
     * evaluate lhs and rhs operands and check types
     */
    void eval_operand(Context&, RefPtr<Variant>&, bool isLH);

    void eval_operands(Context&, RefPtr<Variant>&, RefPtr<Variant>&);

    void throw_invalid_types();

    void set_int_type(const Variant&, const Variant&);

    void try_overloaded_operator(Context&);

    void try_standalone_operator(Context&);

    void try_standalone_operator(Context&, const char*);

    void clear();

public:
    virtual ~BinaryExpr() throw();

    RefPtr<Expr> lhs() const;
    RefPtr<Expr> rhs() const;

private:
    RefPtr<Expr> lhs_;
    RefPtr<Expr> rhs_;
};


#endif // BINARY_EXPR_H__3635E1B0_AE6D_4C0D_ABAF_7A9A3C06914D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
