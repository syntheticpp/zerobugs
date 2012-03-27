//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/breakpoint.h"
#include "breakpoint_view.h"

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
    breakpoints_.push_back(UserBreakPoint{ current_.ref_ptr(), action });
}

