//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/breakpoint.h"
#include "breakpoint_view.h"

using namespace std;
using namespace ui;


BreakPointView::BreakPointView(Controller& controller)
    : View(controller)
{
}


BreakPointView::~BreakPointView() throw()
{
}


void BreakPointView::update(const ui::State&)
{
    breakpoints_.clear();
}


void BreakPointView::update_breakpoint(BreakPoint& bp)
{
    current_ = &bp;
    bp.enum_actions("USER", this);
}


void BreakPointView::notify(BreakPointAction* action)
{
    UserBreakPoint ubp { current_.ref_ptr(), action };

    if (ubp.bpoint)
    {
        index_[ ubp.bpoint->addr() ] = breakpoints_.size();
        breakpoints_.push_back(ubp);
    }
}


UserBreakPoint BreakPointView::addr_to_breakpoint(addr_t addr) const
{
    auto iter = index_.find( addr );

    if (iter == index_.end())
    {
        throw out_of_range(__func__ + string(": address is out of range"));
    }

    assert(iter->second < breakpoints_.size());
    return breakpoints_[iter->second];
}

