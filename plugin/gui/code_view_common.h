#ifndef CODE_VIEW_COMMON_H__43F01FA0_A41D_464E_95DE_2E623C3F61EC
#define CODE_VIEW_COMMON_H__43F01FA0_A41D_464E_95DE_2E623C3F61EC
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

#include <iomanip>
#include <fstream>
#include <sstream>
#include "scope_helpers.h"
#include "code_view.h"
#include "generic/temporary.h"
#include "zdk/zero.h"


/**
 * A callback sink, receives notify() calls from
 * BreakPointManager::enum_breakpoints.
 * Whenever a new file is brought into view, we need
 * to enumerate the breakpoints (so that we can show
 * them in a different color).
 */
class ZDK_LOCAL EnumBreakPoints : public EnumCallback<volatile BreakPoint*>
{
    size_t count_;

public:
    explicit EnumBreakPoints(CodeView& view) : count_(0), view_(&view)
    { }
    EnumBreakPoints() : count_(0), view_(NULL)
    { }

    void notify(volatile BreakPoint* bpnt)
    {
        assert(bpnt);

        if (bpnt->enum_actions("USER"))
        {
            if (view_)
            {
                if (RefPtr<Symbol> sym = bpnt->symbol())
                {
                    view_->on_insert_breakpoint(sym, bpnt->is_deferred());
                }
            }
            ++count_;
        }
    }

    size_t count() const { return count_; }

private:
    CodeView* view_;
};

#endif // CODE_VIEW_COMMON_H__43F01FA0_A41D_464E_95DE_2E623C3F61EC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
