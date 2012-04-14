//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/breakpoint_util.h"
#include "zdk/switchable.h"
#include "zdk/symbol.h"
#include "breakpoint_view.h"
#include "flbreakpoint_table.h"
#include "icons/checkedbox.xpm"
#include "icons/uncheckedbox.xpm"
#include <FL/fl_draw.H>
#include <FL/Fl_Pixmap.H>

using namespace std;


static vector<const char*> header = {
    "", "File", "Line", "Address", "Condition"
};

static const int check_mark_size = 22;


Fl_BreakPointTable::Fl_BreakPointTable(
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
    col_resize_min(check_mark_size);
    col_width(COL_Pixmap, check_mark_size);
    col_width(COL_File, 200);
    col_width(COL_Line, 40);
    col_width(COL_Address, 100);
    col_width(COL_Condition, 1000);

    callback(event_callback, this);
    when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
}


void
Fl_BreakPointTable::draw_cell(
    TableContext ctxt,
    int          row,
    int          col,
    int          x,
    int          y,
    int          w,
    int          h )
{
    ui::UserBreakPoint  ubp;
    RefPtr<Symbol>      sym;

    if (row >= 0 && size_t(row) < view_->size())
    {
        ubp = (*view_)[row];
        sym = ubp.bpoint->symbol();
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
            if (ubp.bpoint.is_null())
            {
                break;
            }
            if (has_enabled_actions(*ubp.bpoint))
            {
                checked_->draw(x, y);
            }
            else
            {
                unchecked_->draw(x, y);
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
            if (ubp.bpoint)
            {
                ostringstream s;
                s << hex << ubp.bpoint->addr();
                fl_draw(s.str().c_str(), x, y, w, h, FL_ALIGN_LEFT);
            }
            break;

        case COL_Condition:
            if (auto s = interface_cast<Switchable*>(ubp.action.get()))
            {
                // breakpoint condition
                const char* expr = s->activation_expr();

                fl_draw(expr, x, y, w, h, FL_ALIGN_LEFT);
            }
            break;
        }

        fl_pop_clip();
        break;

    default:
        break;
    }
}


void
Fl_BreakPointTable::event_callback( Fl_Widget* w, void* v )
{
    reinterpret_cast<Fl_BreakPointTable*>(w)->event_callback();
}


void
Fl_BreakPointTable::event_callback( )
{
    if (callback_context() == CONTEXT_RC_RESIZE)
    {
        // disallow resizing of first column
        // where the check "buttons" are shown
        col_width(COL_Pixmap, check_mark_size);
    }
    else if (eventCallback_)
    {
        eventCallback_();
    }
}

