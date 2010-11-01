#ifndef CAST_EXPR_H__22A59525_924E_4ADD_A8A5_57C11F976140
#define CAST_EXPR_H__22A59525_924E_4ADD_A8A5_57C11F976140
//
// $Id: cast_expr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "expr.h"

class CallSetup;
class ClassType;


/**
 * Models a C-style cast
 */
CLASS CastExpr : public Expr
{
public:
    ///@note conention: the type we're casting to is first,
    ///same as in C/C++ cast expressions the `to' type comes first
    CastExpr(RefPtr<Expr> to, RefPtr<Expr> from, bool constCastOK = true);

    bool is_identity() const
    {
        return conv_ == CAST_IDENTITY;
    }

    bool is_cv_qualified() const
    {
        return conv_ == CAST_CV_QUALIFIED;
    }

    bool is_derived_to_base() const
    {
        return conv_ == CAST_DERIVED_TO_BASE;
    }

    bool is_base_to_derived() const
    {
        return conv_ == CAST_BASE_TO_DERIVED;
    }

    bool is_user_conversion() const
    {
        return conv_ == CAST_USER_CONVERSION;
    }

    bool is_const_cast() const
    {
        return conv_ == CAST_CONST_CAST;
    }

    bool is_dynamic() const { return isDynamic_; }

    bool is_implicit() const { return conv_ == CAST_IMPLICT; }

    enum Conversion
    {
        CAST_NONE,
        CAST_IDENTITY,
        CAST_CV_QUALIFIED,
        CAST_CONST_CAST,
        CAST_DERIVED_TO_BASE,
        CAST_BASE_TO_DERIVED,
        CAST_USER_CONVERSION,
        CAST_IMPLICT
    };

protected:
BEGIN_INTERFACE_MAP(cast)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Variant> eval_sub_expressions(Context&);

    RefPtr<Expr> cast_to() const { return castTo_; }

    RefPtr<Expr> cast_from() const { return castFrom_; }

    RefPtr<Expr> clone(Interp* interp) const
    {
        //return new CastExpr(cast_to(), cast_from(), constCastOK_);
        return new CastExpr(cast_to()->clone(interp),
                            cast_from()->clone(interp),
                            constCastOK_);
    }

    Conversion try_id_and_qualified(
                                const RefPtr<DataType>& typeTo,
                                const RefPtr<DataType>& typeFrom,
                                bool constCastOK) const;

    bool try_qualified(RefPtr<DataType> typeTo,
                       RefPtr<DataType> typeFrom) const;

    /// look for user-defined conversion operators and
    /// conversion (implicit) constructors, and apply them
    /// if found
    bool try_conversion(Context&, DataType& toType);

    bool try_base_and_derived(  Context& context,
                                RefPtr<Variant>& var,
                                PointerType* ptrTo,
                                PointerType* ptrFrom);

    RefPtr<CallSetup> find_conv_ctor(Context&, ClassType&);

    RefPtr<CallSetup> find_conv_operator(
                                Context&,
                                ClassType&,
                                DataType&);

    virtual void throw_invalid_cast(const std::string&);

    RefPtr<Variant> variant_convert(RefPtr<Variant>&) const;

private:
    RefPtr<Expr>    castTo_;
    RefPtr<Expr>    castFrom_;
    Conversion      conv_;
    bool            constCastOK_;
    bool            isDynamic_;
};


/**
 * Models a C++ static_cast
 */
CLASS StaticCast : public CastExpr
{
public:
    StaticCast(RefPtr<Expr>, RefPtr<Expr>);

protected:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        // return new StaticCast(cast_to(), cast_from());
        return new StaticCast(cast_to()->clone(interp),
                              cast_from()->clone(interp)); }

    virtual void throw_invalid_cast(const std::string&);
};


/**
 * Same as static_cast, with the difference that it
 * disallows casts from base to derived
 */
CLASS ImplicitCast : public StaticCast
{
public:
    ImplicitCast(RefPtr<Expr>, RefPtr<Expr>);

private:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        // return new ImplicitCast(cast_to(), cast_from());
        return new ImplicitCast(cast_to()->clone(interp),
                                cast_from()->clone(interp));
    }

    // virtual void throw_invalid_cast(const std::string&);

BEGIN_INTERFACE_MAP(implicit_cast)
    INTERFACE_ENTRY_INHERIT(StaticCast)
END_INTERFACE_MAP()
};


CLASS ConstCast : public CastExpr
{
public:
    ConstCast(RefPtr<Expr> to, RefPtr<Expr> from);

private:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        // return new ConstCast(cast_to(), cast_from());
        return new ConstCast(cast_to()->clone(interp),
                             cast_from()->clone(interp));
    }

BEGIN_INTERFACE_MAP(const_cast)
    INTERFACE_ENTRY_INHERIT(CastExpr)
END_INTERFACE_MAP()
};



CLASS DynamicCast : public CastExpr
{
public:
    DynamicCast(RefPtr<Expr> to, RefPtr<Expr> from);

private:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        // return new DynamicCast(cast_to(), cast_from());
        return new DynamicCast(cast_to()->clone(interp),
                               cast_from()->clone(interp));
    }

BEGIN_INTERFACE_MAP(dynamic_cast)
    INTERFACE_ENTRY_INHERIT(CastExpr)
END_INTERFACE_MAP()
};


CLASS ReinterpretCast : public CastExpr
{
public:
    ReinterpretCast(RefPtr<Expr> to, RefPtr<Expr> from);

private:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        // return new ReinterpretCast(cast_to(), cast_from());
        return new ReinterpretCast(cast_to()->clone(interp),
                                   cast_from()->clone(interp));
    }

BEGIN_INTERFACE_MAP(reinterpret_cast)
    INTERFACE_ENTRY_INHERIT(CastExpr)
END_INTERFACE_MAP()
};

#endif // CAST_EXPR_H__22A59525_924E_4ADD_A8A5_57C11F976140
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
