//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/breakpoint.h"
#include "flbreakpoint_view.h"


FlBreakPointView::FlBreakPointView(

    ui::Controller& c,
    int             x,
    int             y,
    int             w,
    int             h)

: base_type(c, this, x, y, w, h, "Breakpoints")
{
}


FlBreakPointView::~FlBreakPointView() throw()
{
}


void FlBreakPointView::update_breakpoint(BreakPoint& bp)
{
    base_type::update_breakpoint(bp);
    widget()->rows(size());
}

