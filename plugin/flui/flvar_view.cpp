//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/data_type.h"
#include "controller.h"
#include "flvar_view.h"
#include "icons/tree_expanded.xpm"
#include "icons/tree_unexpanded.xpm"
#include <algorithm>
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
//#include <iostream>
using namespace std;

static std::vector<const char*> header = { "Variable", "Value", "Type" };

static const int pix_width = 16;
static const int min_width = 40;


Fl_VarTable::Fl_VarTable(
    ui::VarView*    v,
    int             x,
    int             y,
    int             w,
    int             h,
    const char*     label)

    : Fl_Table(x, y, w, h, label)
    , view_(v)

{
    color(FL_WHITE);
    labelfont(FL_HELVETICA);

    col_header(true);
    col_resize(true);

    cols(header.size());

    col_width(COL_VarName,  200);
    col_width(COL_VarValue, 300);
    //col_width(COL_VarType,  200);
    col_resize_min(min_width);

    callback(event_callback, this);
    when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
}


void Fl_VarTable::draw_cell(

    TableContext    context,
    int             row,
    int             col,
    int             x,
    int             y,
    int             w,
    int             h)
{
    switch (context)
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

        draw_symbol(row, col, x, y, w, h);
        fl_pop_clip();
        break;

    default:
        break;
    }
}


void Fl_VarTable::draw_symbol(
    int row,
    int col,
    int x,
    int y,
    int w,
    int h )
{
    if (size_t(row) >= view_->variable_count())
    {
        return;
    }

    DebugSymbol& sym = view_->get_variable(row);
    switch (col)
    {
    case COL_VarName:
        x += pix_width * sym.depth();
        if (sym.enum_children())
        {
            const char* const* pixmap = view_->is_expanding(row)
                ? tree_expanded_xpm
                : tree_unexpanded_xpm;

            fl_draw_pixmap(pixmap, x, y + 4, FL_WHITE);
        }
        fl_draw(sym.name()->c_str(), x + pix_width, y, w, h, FL_ALIGN_LEFT);
        break;

    case COL_VarValue:
        if (view_->has_variable_changed(sym))
        {
            fl_color(FL_RED);
        }
        if (auto val = sym.value())
        {
            fl_draw(val->c_str(), x + 2, y, w, h, FL_ALIGN_LEFT);
        }
        break;

    case COL_VarType:
        if (auto type = sym.type())
        {
            fl_draw(type->name()->c_str(), x + 2, y, w, h, FL_ALIGN_LEFT);
        }
        break;
    }
}


void Fl_VarTable::event_callback(Fl_Widget* w, void*)
{
    reinterpret_cast<Fl_VarTable*>(w)->event_callback();
}


void Fl_VarTable::event_callback()
{
    if (callback_context() == CONTEXT_RC_RESIZE)
    {
        resize(w());
        return;
    }

    const size_t row = callback_row();
    if (row >= view_->variable_count())
    {
        return;
    }

    if (callback_col() == COL_VarName)
    {
        // check if user clicked on the tree expand / unexpand icon
        DebugSymbol& sym = view_->get_variable(row);
        const int x_min = pix_width * sym.depth() + x();
        const int x_max = x_min + pix_width + x();

        if (Fl::event() == FL_PUSH && Fl::event_button1() &&
            Fl::event_x() > x_min && Fl::event_x() <= x_max)
        {
            const bool expanded = view_->is_expanding(row);

            view_->expand(row, !expanded);
        }
    }
}


void Fl_VarTable::resize(int w)
{
    int width = w - col_width(COL_VarName) - col_width(COL_VarValue) - 2;
    if (width > min_width)
    {
        col_width(COL_Last, width);
    }
}


void Fl_VarTable::resize(int x, int y, int w, int h)
{
    Fl_Table::resize(x, y, w, h);
    resize(w);
}


////////////////////////////////////////////////////////////////
FlVarView::FlVarView(ui::Controller& c)
    : base_type(c, this, 0, 0, 0, 0, nullptr)
{
}


////////////////////////////////////////////////////////////////
FlLocalsView::FlLocalsView(ui::Controller& c, const char* label)
    : base_type(c, this, 0, 0, 0, 0, label)
{
    widget()->tooltip("Local variables");
}


void FlLocalsView::update(const ui::State& state)
{
    base_type::update(state);
    widget()->rows(variable_count());
}

