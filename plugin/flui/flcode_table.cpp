//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flcode_table.h"
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <iostream>


FlCodeTable::FlCodeTable(int x, int y, int w, int h, const char* label)
    : Fl_Table(x, y, w, h, label)
{
    cols(3);
    col_width(0, 20);
    col_width(2, 1000);
    // just playing around for now...
    rows(10000);
}


// hacked example from:
// http://seriss.com/people/erco/Fl_Table/documentation/Fl_Table.html#draw_cell
void FlCodeTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
    static char s[40];
    switch  (C)
    {
    case 0: s[0] = 0; break;
    case 1: sprintf(s, "%05d", R); break;
    case 2: sprintf(s, "/* some code line goes here */"); break;
    }

    switch ( context )
    {
    case CONTEXT_STARTPAGE:             // Fl_Table telling us its starting to draw page
        damage(FL_DAMAGE_ALL);
        fl_font(FL_COURIER, 11);
        return;

    case CONTEXT_ROW_HEADER:            // Fl_Table telling us it's draw row/col headers
    case CONTEXT_COL_HEADER:
        fl_push_clip(X, Y, W, H);
        {
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, color());
            fl_color(FL_BLACK);
            fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
        }
        fl_pop_clip();
        return;

    case CONTEXT_RC_RESIZE:
        return;

    case CONTEXT_CELL:                  // Fl_Table telling us to draw cells
        fl_push_clip(X, Y, W, H);
        {
            // BG COLOR
            if (C == 0)
            {
                fl_color( /*row_selected(R) ? selection_color() : */ FL_LIGHT2);
            }
            else
            {
                fl_color( /*row_selected(R) ? selection_color() : */ FL_WHITE);
            }
            fl_rectf(X, Y, W, H);

            // TEXT
            fl_color(FL_BLACK);
            fl_draw(s, X, Y, W, H, FL_ALIGN_LEFT);

            // BORDER
            //fl_color(FL_LIGHT2); 
            //fl_rect(X, Y, W, H);
        }
        fl_pop_clip();
        return;

    case CONTEXT_ENDPAGE:
    default:
        return;
    }
    //NOTREACHED
}
