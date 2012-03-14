//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/symbol.h"
#include "breakpoint_view.h"
#include "flbreakpoint_table.h"
#include "icons/checkedbox.xpm"
#include "icons/uncheckedbox.xpm"
#include <FL/fl_draw.H>
#include <FL/Fl_Pixmap.H>

using namespace std;


static vector<const char*> header = { "", "File", "Line", "Address" };

enum ColType
{
    COL_Pixmap,
    COL_File,
    COL_Line,
    COL_Address
};


FlBreakPointTable::FlBreakPointTable(
    View*       v,
    int         x,
    int         y,
    int         w,
    int         h,
    const char* label )

    : Fl_Table_Row( x, y, w, h, label )
    , view_(v)
    , checked_(new Fl_Pixmap(checkedbox_xpm))
    , unchecked_(new Fl_Pixmap(uncheckedbox_xpm))
{
    color(FL_WHITE);
    labelfont(FL_HELVETICA);

    col_header(true);
    col_resize(true);

    cols(header.size());
    col_resize_min(20);
    col_width(COL_Pixmap, 20);
    col_width(COL_File, 200);
    col_width(COL_Line, 40);
    col_width(COL_Address, 1000);
}


void
FlBreakPointTable::draw_cell(
    TableContext ctxt,
    int          row,
    int          col,
    int          x,
    int          y,
    int          w,
    int          h )
{
    RefPtr<BreakPoint>  bp;
    RefPtr<Symbol>      sym;

    if (row >= 0 && row < view_->size())
    {
        bp  = (*view_)[row];
        sym = bp->symbol();
    }

    switch (ctxt)
    {
    case CONTEXT_COL_HEADER:
        fl_font(FL_HELVETICA_BOLD, 11);
        fl_push_clip(x, y, w, h);
        fl_color(FL_LIGHT3);
        fl_draw_box(FL_THIN_UP_BOX, x, y, w, h, FL_LIGHT2);
        fl_color(FL_BLACK);
        fl_draw(header[col], x + 2, y, w, h, FL_ALIGN_LEFT);
        fl_pop_clip();
        break;

    case CONTEXT_CELL:
        fl_font(FL_HELVETICA, 11);
        fl_push_clip(x, y, w, h);
        fl_draw_box(FL_BORDER_BOX, x, y, w, h, FL_WHITE);
        fl_frame("XXRR", x, y, w, h);
        fl_color(FL_BLACK);
        switch (col)
        {
        case COL_Pixmap:
            if (bp)
            {
                if (bp->is_enabled())
                {
                    checked_->draw(x, y);
                }
                else
                {
                    unchecked_->draw(x, y);
                }
            }
            break;

        case COL_File:
            if (sym)
            {
                fl_draw(basename(sym->file()->c_str()), x, y, w, h, FL_ALIGN_LEFT);
            }
            break;

        case COL_Line:
            if (sym && sym->line())
            {
                ostringstream s;
                s << sym->line();
                fl_draw(s.str().c_str(), x, y, w, h, FL_ALIGN_LEFT);
            }
            break;

        case COL_Address:
            if (bp)
            {
                ostringstream s;
                s << hex << bp->addr();
                fl_draw(s.str().c_str(), x, y, w, h, FL_ALIGN_LEFT);
            }
            break;
        }
        fl_pop_clip();
        break;

    default:
        break;
    }
}

