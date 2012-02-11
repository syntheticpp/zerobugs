//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/stack.h"
#include "zdk/symbol_table.h"
#include "zdk/zobject_scope.h"
#include "flstack_view.h"
#include <FL/fl_draw.H>


static const std::vector<const char*> headers = {
    "Address", "Function", "Line", "Path", "Module"
};

static const int LEFT_MARGIN = 2;


Fl_StackTable::Fl_StackTable(

    ui::StackView*  view,
    int             x,
    int             y,
    int             w,
    int             h,
    const char*     label )

    : Fl_Table_Row(x, y, w, h, label)
    , view_(view)
{
    cols(headers.size());

    col_width(0, 100);
    col_width(1, 200);
    col_width(2, 40);
    col_width(3, 800);
    col_width(4, 1000);
    col_resize_min(40);

    col_header(true);
    col_resize(true);

    color(FL_WHITE);
    labelfont(FL_HELVETICA);

    callback(event_callback, this);
    when(FL_WHEN_RELEASE);
}


void Fl_StackTable::draw_cell(

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
        fl_color(FL_LIGHT2);
        fl_draw_box(FL_THIN_UP_BOX, x, y, w, h, FL_LIGHT2);
        fl_color(FL_BLACK);
        fl_draw(headers[col], x + LEFT_MARGIN, y, w, h, FL_ALIGN_LEFT);
        fl_pop_clip();
        break;

    case CONTEXT_CELL:
        {
            fl_font(FL_HELVETICA, 10);
            fl_push_clip(x, y, w, h);
            const Fl_Color color = row_selected(row) ? FL_BACKGROUND_COLOR : FL_WHITE;

            fl_draw_box(FL_BORDER_BOX, x, y, w, h, color);
            fl_frame("XXRR", x, y, w, h);
            fl_color(FL_BLACK);

            draw_frame(row, col, x, y, w, h);
            fl_pop_clip();
        }
        break;

    default:
        break;
    }
}


void Fl_StackTable::draw_frame(

    int row,
    int col,
    int x,
    int y,
    int w,
    int h)
{
    if (size_t(row) >= view_->frame_count())
    {
        return;
    }

    RefPtr<Frame> frame = view_->get_frame(row);
    Symbol* sym = frame->function();

    switch (col)
    {
    case 0: // address
        {
            std::ostringstream ss;
            ss << "0x" << std::hex << frame->program_count();
            fl_draw(ss.str().c_str(), x + LEFT_MARGIN , y, w, h, FL_ALIGN_LEFT);
        }
        break;

    case 1: // function
        if (sym)
        {
            fl_draw(sym->name()->c_str(), x + LEFT_MARGIN , y, w, h, FL_ALIGN_LEFT);
        }
        break;

    case 2: // line
        if (sym)
        {
            if (sym->line())
            {
                std::ostringstream ss;
                ss << sym->line();
                fl_draw(ss.str().c_str(), x + LEFT_MARGIN , y, w, h, FL_ALIGN_LEFT);
            }
        }
        break;

    case 3: // path
        if (sym)
        {
            fl_draw(sym->file()->c_str(), x + LEFT_MARGIN , y, w, h, FL_ALIGN_LEFT);
        }
        break;

    case 4: // module
        if (sym)
        {
            ZObjectScope scope;
            if (auto table = sym->table(&scope))
            {
                fl_draw(table->filename()->c_str(), x + LEFT_MARGIN , y, w, h, FL_ALIGN_LEFT);
            }
        }
        break;
    }
}


void Fl_StackTable::resize(int x, int y, int w, int h)
{
    Fl_Table_Row::resize(x, y, w, h);
}


void Fl_StackTable::event_callback(Fl_Widget* w, void*)
{
    reinterpret_cast<Fl_StackTable*>(w)->event_callback();
}


void Fl_StackTable::event_callback()
{
    if (Fl::event_button1() && Fl::event_is_click())
    {
        select_row(callback_row(), 1);
    }
}


////////////////////////////////////////////////////////////////
FlStackView::FlStackView(ui::Controller& c) 
    : base_type(c, this, 0, 0, 0, 0, "Stack")
{
}


void FlStackView::update(const ui::State& state)
{
    base_type::update(state);
    widget()->rows(frame_count());
    widget()->redraw();
}

