#ifndef POSTFIX_EXPR_H__A14A3FA5_1071_4E2C_A5E5_D240AA766231
#define POSTFIX_EXPR_H__A14A3FA5_1071_4E2C_A5E5_D240AA766231
//
// $Id: postfix_expr.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "context.h"
#include "expr.h"


CLASS PostfixExpr : public Expr, public MacroHelper
{
public:
    DECLARE_UUID("795b9453-c322-4062-8a37-573673c22207")

    enum Operator
    {
        ARRAY,          // operator[]
        FCALL,          // operator()
        MEMBER,         // foo.bar
        POINTER,        // foo->bar
        INC,            // post-increment
        DEC,            // post-decrement
        PTR_TO_MEMBER,

        // --- not part of C/C++, but useful:
        VPOINTER,       // =>, derefs to most derived
        ARANGE,         // [ : ]
    };

    PostfixExpr
    (
        Interp*,
        Operator,
        const RefPtr<Expr>&,
        const RefPtr<Expr>& param
    );

    virtual ~PostfixExpr() throw();

    Operator oper() const { return oper_; }

    RefPtr<Expr> clone(Interp*) const;

protected:
    friend class PointerExprHelper;

    RefPtr<Expr> param() const { return param_; }
    RefPtr<Ident> ident() const { return ident_; }

    RefPtr<DebugSymbol> eval_pointer(
        Context&, const RefPtr<Expr>&, const RefPtr<Variant>&);

    RefPtr<Variant> eval_member(
        Context&, DebugSymbol&, const std::string&, bool = false);

    virtual void set_ptr(const RefPtr<Expr>& ptr) { ptr_ = ptr; }

    void check_for_operator_arrow(Context&, RefPtr<Variant>);

protected:
BEGIN_INTERFACE_MAP(PostfixExpr)
    INTERFACE_ENTRY(PostfixExpr)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    RefPtr<Expr> expr() const { return expr_; }

    RefPtr<Variant> eval_impl(Context&);

private:
    RefPtr<Variant> eval_member(Context&, const Variant&);
    RefPtr<Variant> eval_ptr_to_member(Context&, const Variant&);

    RefPtr<Variant> eval_array(Context&, const Variant&);
    RefPtr<Variant> eval_array_range(Context&, const Variant&);

    RefPtr<DebugSymbol> eval_pointer(Context&, const Variant&);

    RefPtr<Variant> eval_function_call(Context&, Variant&);

    void eval_function_call(Context&, DebugSymbol&);

    virtual void map_arguments(const std::string&, ArgMap&);

    /**
     * invoked from function_call, attempts to find a method
     * that matches the number and types of call arguments
     */
    RefPtr<CallSetup> get_method(Context&, DebugSymbol&, ExprList*);

    RefPtr<CallSetup>
        get_overloaded_operator(Context&, bool strict = false);

    void initiate_fun_call(Context&, CallSetup&);

private:
    Operator        oper_;
    RefPtr<Expr>    expr_;
    RefPtr<Expr>    param_;
    RefPtr<Ident>   ident_;
    RefPtr<Expr>    ptr_;
    bool            isMacroCall_;
};



/**
 * Models a function call's argument list
 */
CLASS ArgumentList : public Expr
{
public:
    DECLARE_UUID("158ccf37-4614-4097-b52b-d9ef88f48e10")

BEGIN_INTERFACE_MAP(ArgumentList)
    INTERFACE_ENTRY(ArgumentList)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    ArgumentList(Interp*, const RefPtr<Expr>&);

    ~ArgumentList() throw();

    void push_back(RefPtr<Expr> expr)
    {
        //assert(expr.get());
        args_.push_back(expr);
    }

    void push_front(RefPtr<Expr> expr)
    {
        assert(expr.get());
        args_.insert(args_.begin(), expr);
    }

    const ExprList& args() const { return args_; }

    ExprList& args() { return args_; }

protected:
    RefPtr<Variant> eval_impl(Context&);

    ArgumentList(Interp* interp, const ExprList& args)
        : Expr(interp)
    {
        ExprList::const_iterator i = args.begin();
        for (; i != args.end(); ++i)
        {
            args_.push_back((*i)->clone(interp));
        }
    }

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new ArgumentList(interp, args_);
    }

private:
    ExprList args_;
};

#endif // POSTFIX_EXPR_H__A14A3FA5_1071_4E2C_A5E5_D240AA766231
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
