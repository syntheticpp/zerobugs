//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/fl_draw.H>
#include "flvar_view.h"


Fl_VarTable::Fl_VarTable(int x, int y, int w, int h, const char* label)
    : Fl_Table(x, y, w, h, label)

{
    col_header(true);
    col_resize(true);

    cols(3);
    rows(100); 

    col_width(0, 300);
    col_width(1, 400);
    col_width(2, 2000); 
}


void Fl_VarTable::draw_cell(

    TableContext    context,
    int             row,
    int             col,
    int             x,
    int             y,
    int             width,
    int             height)
{
    switch (context)
    {
    case CONTEXT_STARTPAGE:
        //fl_font(font_, fontSize_);
        break;

    case CONTEXT_COL_HEADER:
        fl_push_clip(x, y, width, height);
        fl_color(FL_LIGHT2);
        fl_draw_box(FL_THIN_UP_BOX, x, y, width, height, FL_LIGHT2);
        fl_pop_clip();
        break;

    case CONTEXT_CELL:
        fl_push_clip(x, y, width, height);
        fl_draw_box(FL_BORDER_BOX, x, y, width, height, FL_WHITE);
        fl_frame("XXRR", x, y, width, height);
        fl_pop_clip();
        break;

    default:
        break;
    }
}

