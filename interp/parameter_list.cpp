// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: parameter_list.cpp 714 2010-10-17 10:03:52Z root $
//
#include "errors.h"
#include "parameter_list.h"


ParamDeclList::ParamDeclList(Interp* interp) : Expr(interp)
{
}


ParamDeclList::~ParamDeclList() throw()
{
}


RefPtr<Variant> ParamDeclList::eval_impl(Context&)
{
    throw std::logic_error("Cannot evaluate parameter "
                           "declaration list directly");
}


void ParamDeclList::push(RefPtr<Expr> expr)
{
    assert(expr.get());
    list_.push_back(expr);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
