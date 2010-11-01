#ifndef QUALIFIER_LIST_H__2BE3521D_9068_45AB_A877_45339BAAFF2E
#define QUALIFIER_LIST_H__2BE3521D_9068_45AB_A877_45339BAAFF2E
//
// $Id: qualifier_list.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/types.h"
#include "expr.h"


CLASS QualifierList : public Expr
{
public:
    DECLARE_UUID("66d8695b-f509-45b1-b28a-70324143d47a")
BEGIN_INTERFACE_MAP(QualifierList)
    INTERFACE_ENTRY(QualifierList)
    INTERFACE_ENTRY_INHERIT(Expr)
END_INTERFACE_MAP()

    QualifierList(Interp*, Qualifier);

    ~QualifierList() throw();

    void add(Qualifier qual);

    void add(RefPtr<Expr>);

    Qualifier get() const { return qual_; }

private:
    RefPtr<Variant> eval_impl(Context&);

    RefPtr<Expr> clone(Interp* interp) const
    {
        return new QualifierList(interp, qual_);
    }

    Qualifier qual_;
};
#endif // QUALIFIER_LIST_H__2BE3521D_9068_45AB_A877_45339BAAFF2E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
