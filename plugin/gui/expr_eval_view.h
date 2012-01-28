#ifndef EXPR_EVAL_VIEW_H__59892F09_BB71_4831_972D_4D8B2D670B78
#define EXPR_EVAL_VIEW_H__59892F09_BB71_4831_972D_4D8B2D670B78
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

#include "variables_view.h"


class ZDK_LOCAL ExprEvalView : public VariablesView
{
public:
    explicit ExprEvalView(Debugger&);

    ~ExprEvalView() throw();

protected:
    bool update(RefPtr<Thread>);
};

#endif // EXPR_EVAL_VIEW_H__59892F09_BB71_4831_972D_4D8B2D670B78
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
