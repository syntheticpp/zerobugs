//
// $Id: expr_eval_view.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cassert>
#include <iostream>
#include "generic/lock_ptr.h"
#include "zdk/thread_util.h"
#include "expr_eval_view.h"
#include "gui.h"


using namespace std;


ExprEvalView::ExprEvalView(Debugger& dbg) : VariablesView(dbg)
{
}


ExprEvalView::~ExprEvalView() throw()
{
}


bool ExprEvalView::update(RefPtr<Thread> thread)
{
    if (!VariablesView::update(thread))
    {
        return false;
    }
    DebugSymbolList tmp = this->debug_symbols();
    this->clear_symbols();

    for (DebugSymbolList::iterator i = tmp.begin(); i != tmp.end(); ++i)
    {
        if ((*i)->addr())
        {
            (*i)->read(this);
        }
        this->notify(i->get());
    }
    return true;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
