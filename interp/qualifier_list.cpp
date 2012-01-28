// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <stdexcept>
#include "qualifier_list.h"



QualifierList::QualifierList(Interp* interp, Qualifier qual)
    : Expr(interp)
    , qual_(qual)
{
}


QualifierList::~QualifierList() throw()
{
}


void QualifierList::add(Qualifier qual)
{
    qual_ = static_cast<Qualifier>(qual_ | qual);
}


void QualifierList::add(RefPtr<Expr> expr)
{
    assert(expr.get());
    add(interface_cast<QualifierList&>(*expr).get());
}


RefPtr<Variant> QualifierList::eval_impl(Context&)
{
    assert(false);
    throw std::logic_error("cannot eval qualifier list directly");
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
