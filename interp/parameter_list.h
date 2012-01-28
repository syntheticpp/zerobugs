#ifndef PARAMETER_LIST_H__23DA7E72_FBED_4C50_A9E5_9C04675E5618
#define PARAMETER_LIST_H__23DA7E72_FBED_4C50_A9E5_9C04675E5618
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

CLASS ParamDeclList : public Expr
{
public:
    DECLARE_UUID("827cbb02-f8c7-46f2-957c-b47eef3dee29")

BEGIN_INTERFACE_MAP(ParamDeclList)
    INTERFACE_ENTRY(ParamDeclList)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    explicit ParamDeclList(Interp*);

    ~ParamDeclList() throw();

    const ExprList& list() const { return list_; }

    void push(RefPtr<Expr>);

protected:
    ParamDeclList(Interp* interp, const ExprList& list)
        : Expr(interp)
    {
        ExprList::const_iterator i = list.begin();
        for (; i != list.end(); ++i)
        {
            list_.push_back((*i)->clone(interp));
        }
    }

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new ParamDeclList(interp, list_);
    }

private:
    RefPtr<Variant> eval_impl(Context&);

    ExprList list_;
};
#endif // PARAMETER_LIST_H__23DA7E72_FBED_4C50_A9E5_9C04675E5618
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
