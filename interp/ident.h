#ifndef IDENT_H__58BDEE5A_7DF3_46D5_888E_F3DEBF15078E
#define IDENT_H__58BDEE5A_7DF3_46D5_888E_F3DEBF15078E
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
//
#include <string>
#include "zdk/enum.h"
#include "zdk/register.h"
#include "primary_expr.h"

/**
 * Models a language identifier
 */
CLASS Ident : public PrimaryExpr, EnumCallback<Register*>
{
public:
DECLARE_UUID("06b3484e-8c2d-4c06-aff7-6ddd3bb18840")

BEGIN_INTERFACE_MAP(Ident)
    INTERFACE_ENTRY(Ident)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    Ident(Interp*, const char*, bool func = false, Register* = 0);

    Ident(Interp*, const char*, RefPtr<DebugSymbol>);

    /**
     * ctor for fully-qualified names
     */
    Ident(RefPtr<Expr> prefix, RefPtr<Expr> ident);

    ~Ident() throw();

    /**
     * @return the qualified name
     */
    const std::string& name() const { return name_; }

    const std::string& alt_name() const { return altName_; }

    /**
     * By default, if this identifies a reference type,
     * the result of evaluating it is the referred object.
     * The default behavior may need to be disabled when
     * evaluating as part of a cast expression.
     */
    void disable_auto_deref() { autoDeref_ = false; }

protected:
    RefPtr<Variant> eval_impl(Context&);
    RefPtr<Variant> eval_reg(Context&);

    void notify(Register*);

    RefPtr<Expr> clone(Interp* interp) const
    {
        if (!reg_)
        {
            return new Ident(interp, altName_.c_str(), sym_);
        }
        else
        {
            return new Ident(interp, name_.c_str(), func_, reg_.get());
        }
    }

private:
    std::string         name_;
    std::string         altName_;
    RefPtr<DebugSymbol> sym_;
    RefPtr<Register>    reg_;
    bool                autoDeref_;
    bool                func_;
};

#endif // IDENT_H__58BDEE5A_7DF3_46D5_888E_F3DEBF15078E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
