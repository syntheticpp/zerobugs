#ifndef AUTOCONTEXT_H__7D3CF628_13ED_4790_B589_338C313B03F6
#define AUTOCONTEXT_H__7D3CF628_13ED_4790_B589_338C313B03F6
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include "zdk/zero.h"


namespace
{
    /**
     * Helper, automatically pops the action context when
     * going out of scope.
     *
     * @see BreakPointBase::execute_actions
     */
    class ZDK_LOCAL AutoContext
    {
        AutoContext(const AutoContext&);
        AutoContext& operator=(const AutoContext&);


        Debugger* dbg_;
        ZObject*  ctxt_;

    public:
        AutoContext(Thread* thread, ZObject* ctxt)
            : dbg_(NULL)
            , ctxt_(ctxt)
        {
            if (thread)
            {
                dbg_ = thread->debugger();
            }
            if (dbg_ && ctxt_)
            {
                dbg_->push_action_context(ctxt_);
            }
        }

        ~AutoContext()
        {
            if (dbg_ && ctxt_)
            {
                dbg_->pop_action_context();
            }
        }
    };
}

#endif // AUTOCONTEXT_H__7D3CF628_13ED_4790_B589_338C313B03F6
