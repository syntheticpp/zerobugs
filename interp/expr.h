#ifndef EXPR_H__92614E0E_EE20_469C_955D_E71E291B88B6
#define EXPR_H__92614E0E_EE20_469C_955D_E71E291B88B6
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

#include <vector>
#include <map>
#include "zdk/data_type.h"
#include "zdk/variant.h"
#include "zdk/weak_ptr.h"
#include "zdk/zobject_impl.h"

class BreakPointAction;
class CallSetup;
class Context;
class Interp;
class PostfixExpr;
class Thread;
class TypeInfo;


CLASS Expr : public ZObjectImpl<>
{
public:
    DECLARE_UUID("79d96109-8f44-40ac-8a33-6d530af6d08d")

BEGIN_INTERFACE_MAP(Expr)
    INTERFACE_ENTRY(Expr)
END_INTERFACE_MAP()

    /**
     * @return the type of this expression
     */
    RefPtr<DataType> type() const;

    RefPtr<Variant> eval(Context&);

    RefPtr<Variant> value() const { return value_; }

    /**
     * @return the interpretor associated with this expr
     */
    RefPtr<Interp> interp() const;

    RefPtr<BreakPointAction> new_call_return_action(
        RefPtr<Symbol>,
        CallSetup&,
        Thread&,
        DebugInfoReader*);

    void set_type(const RefPtr<DataType>&);

    virtual void set_result(const RefPtr<Variant>&);

    void set_strict_type(bool strict) { strictType_ = strict; }

    bool const is_type_strict() const { return strictType_; }

    RefPtr<CallSetup> call_setup() const;

    virtual RefPtr<Expr> clone(Interp*) const = 0;

protected:
    virtual ~Expr() throw();

    explicit Expr(Interp*);

    /**
     * Evaluate the expression in the given context.
     * @note: template method pattern
     */
    virtual RefPtr<Variant> eval_impl(Context&) = 0;

    RefPtr<Variant> expand_macro(Context&);

private:
    WeakPtr<DataType>   type_;
    RefPtr<Variant>     value_;
    WeakPtr<Interp>     interp_;
    bool                strictType_;
    RefPtr<CallSetup>   callSetup_;
};


typedef std::vector<RefPtr<Expr> > ExprList;

#endif // EXPR_H__92614E0E_EE20_469C_955D_E71E291B88B6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
