#ifndef ADDITIVE_EXPR_H__E7EE8C97_C9CD_42F5_A3D5_5DE313302995
#define ADDITIVE_EXPR_H__E7EE8C97_C9CD_42F5_A3D5_5DE313302995
//
// $Id: additive_expr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "binary_expr.h"

class VariantImpl; // forward

/**
 * Evaluates additions and subtractions
 */
CLASS AdditiveExpr : public BinaryExpr
{
BEGIN_INTERFACE_MAP(AdditiveExpr)
    INTERFACE_ENTRY_INHERIT(BinaryExpr)
END_INTERFACE_MAP()

public:
    enum Operator { PLUS = '+', MINUS = '-' };

    AdditiveExpr(Interp*, Operator, RefPtr<Expr> lhs, RefPtr<Expr> rhs);

protected:
    RefPtr<Variant> eval_impl(Context&);

    const char* operator_name() const;

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new AdditiveExpr(interp,
                                operator_,
                                lhs()->clone(interp),
                                rhs()->clone(interp));
    }

private:
    void add_float_int(
        const Variant& lhs,
        const Variant& rhs,
        Variant& result,
        bool negate = false);

    void add_uint64_int(
        const Variant& lhs,
        const Variant& rhs,
        Variant& result,
        bool negate = false);

    void add_integers(
        const Variant& lhs,
        const Variant& rhs,
        Variant& result);

    void add_floats(
        const Variant& lhs,
        const Variant& rhs,
        Variant& result);

    /**
     * pointer arithmetic, increment pointer
     */
    void add_pointer_int(
        const Variant& lhs,
        const Variant& rhs,
        VariantImpl& result,
        DataType& pointerType);

private:
    Operator operator_;
};
#endif // ADDITIVE_EXPR_H__E7EE8C97_C9CD_42F5_A3D5_5DE313302995
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
