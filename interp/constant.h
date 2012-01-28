#ifndef CONSTANT_H__2D02EAF4_DBA5_4E3C_AE10_B2BA0C600D87
#define CONSTANT_H__2D02EAF4_DBA5_4E3C_AE10_B2BA0C600D87
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

#include "primary_expr.h"
#include "typez/public/type_tags.h"


CLASS Constant : public PrimaryExpr
{
public:
DECLARE_UUID("fecfa3f0-e167-49f6-82df-e4060af7f26f")

    virtual ~Constant() throw();

    Constant(Interp*, DebugSymbol&);

protected:
BEGIN_INTERFACE_MAP(Constant)
    INTERFACE_ENTRY(Constant)
    INTERFACE_ENTRY_INHERIT(PrimaryExpr)
END_INTERFACE_MAP()

    explicit Constant(Interp*);

    Constant(Interp* interp, const RefPtr<Variant>& var)
        : PrimaryExpr(interp)
        , var_(var)
    { }

    void set_variant(Variant& var);

    template<typename T>
    void set_value(T value, Variant::TypeTag tag)
    {
        put(var_.get(), value, tag);
    }

    const RefPtr<Variant>& var() const { return var_; }

    RefPtr<Variant> eval_impl(Context&) { return var_; }

    RefPtr<Expr> clone(Interp*) const;

private:
    RefPtr<Variant> var_;
};


CLASS IntegerConstant : public Constant
{
public:
    /// construct a constant from the current token
    IntegerConstant(Interp*, const char* yytext, int base);

    /// construct a character constant
    IntegerConstant(Interp*, char);

    virtual ~IntegerConstant() throw();
};


CLASS FloatingPointConstant : public Constant
{
public:
    /// construct a constant from the current token
    FloatingPointConstant(Interp*, const char* yytext);

    virtual ~FloatingPointConstant() throw();
};


CLASS StringLiteralConstant : public Constant
{
public:
    StringLiteralConstant(Interp*, const char*);

    virtual ~StringLiteralConstant() throw();
};
#endif // CONSTANT_H__2D02EAF4_DBA5_4E3C_AE10_B2BA0C600D87
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
