//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flbreakpoint_table.h"

FlBreakPointTable::FlBreakPointTable(
    int         x,
    int         y,
    int         w,
    int         h,
    const char* label )

    : Fl_Table_Row( x, y, w, h, label )
{
}


void
FlBreakPointTable::draw_cell(

    TableContext    ctxt,
    int             row,
    int             col,
    int             x,
    int             y,
    int             w,
    int             h)
{
}

