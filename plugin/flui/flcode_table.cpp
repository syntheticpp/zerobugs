//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/stdexcept.h"
#include "zdk/symbol.h"
#include "flcode_table.h"
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <algorithm>
#include <cassert>
#include <boost/algorithm/string.hpp>

using namespace std;


const string Fl_CodeTable::mark_arrow = { "arrow" };

enum ColumnType
{
    COL_Mark,
    COL_Line,
    COL_Text,
    COL_Asm,
    COL_Addr = COL_Line
};

////////////////////////////////////////////////////////////////

Fl_CodeTable::Fl_CodeTable(int x, int y, int w, int h, const char* label)
    : Fl_Table(x, y, w, h, label)
    , highlight_(0)
{
    col_header(true);
}


Fl_Color Fl_CodeTable::cell_background(int row, int col) const
{
    if (col == COL_Mark)
    {
        return FL_LIGHT2;
    }
    else
    {
        return row + 1 == highlight_ ? highlight_background() : text_background();
    }
}


Fl_Font Fl_CodeTable::font() const
{
    return FL_SCREEN;
}

int Fl_CodeTable::font_size() const
{
    return 11;
}


// Set the highlighted line index. There can be only one.
void Fl_CodeTable::highlight_line(ui::CodeListing* listing, int line)
{
    highlight_ = line;

    if (line)
    {
        --line;

        if (line < 0 || size_t(line) >= listing->line_count())
        {
            throw out_of_range(__func__);
        }

        row_position(line ? line - 1 : line);
    }
}


// draw pixmap marks associated with row, if any
void Fl_CodeTable::draw_line_marks(int row, int x, int y)
{
    auto m = marks_.find(row + 1);
    if (m != marks_.end())
    {
        for (auto mark : m->second)
        {
            auto p = pixmaps_.find(mark);
            if (p != pixmaps_.end())
            {
                fl_draw_pixmap(p->second.cstrings(), x, y, FL_LIGHT2);
            }
        }
    }
}


void Fl_CodeTable::set_mark_at_line(

    int                 line,
    const std::string&  mark,
    bool                setMark /* = true */)
{
    if (setMark)
    {
        marks_[line].insert(mark);
    }
    else
    {
        marks_[line].erase(mark);
    }
}


void Fl_CodeTable::set_mark_pixmap(

    const std::string&  mark,
    const char* const*  data )
{
    pixmaps_.insert(make_pair(mark, data));
}


////////////////////////////////////////////////////////////////
//
// Source Code Table
//
Fl_SourceTable::Fl_SourceTable(

    int         x,
    int         y,
    int         w,
    int         h,
    const char* label)

    : Fl_CodeTable(x, y, w, h, label)
    , listing_(nullptr)
    , digits_(0)
    , maxTextLen_(0)
{
    cols(3);// one column for markers, one for line numbers, third for text
            // (one for the money, two for the show three to get ready...)

    col_width(COL_Mark, 30);
    col_width(COL_Line, 50);
    col_width(COL_Text, 500);
}


void Fl_SourceTable::refresh(

    const RefPtr<Thread>& t,
    const RefPtr<Symbol>& sym)

{
    if (listing_->refresh(t, sym))
    {
        digits_ = 0;
        maxTextLen_ = 0;
        rows(listing_->line_count());
    }

    const size_t line = listing_->current_line();

    set_mark_at_line(highlighted_line(), mark_arrow, false);
    highlight_line(listing_, line);
    set_mark_at_line(line, mark_arrow);
}


// Compute lazily how many digits are needed to print line numbers
int Fl_SourceTable::line_digits() const
{
    if (digits_ == 0)
    {
        size_t n = listing_->line_count();
        while (n)
        {
            n /= 10;
            ++digits_;
        }
    }
    return digits_;
}


void Fl_SourceTable::draw_header(int col, int x, int y, int w, int h)
{
    fl_push_clip(x, y, w, h);

    fl_color(header_background());
    fl_rectf(x, y, w, h);

    if (col == COL_Text)
    {
        fl_color(text_color());
        fl_draw(listing_->current_file(), x, y, w, h, FL_ALIGN_LEFT);
    }

    fl_pop_clip();
}


void Fl_SourceTable::draw_cell(
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
        fl_font(font(), font_size());
        break;

    case CONTEXT_COL_HEADER:
        draw_header(col, x, y, width, height);
        break;

    case CONTEXT_CELL:
        fl_push_clip(x, y, width, height);
        {
            fl_color(cell_background(row, col));
            fl_rectf(x, y, width, height);

            if (col == COL_Mark)
            {
                draw_line_marks(row, x, y);
            }
            else if (col == COL_Line)
            {
                // format and draw line numbers
                char s[16] = { 0 };

                snprintf(s, sizeof(s - 1), "%*d", line_digits(), row + 1);

                fl_color(text_color());
                fl_draw(s, x, y, width, height, FL_ALIGN_LEFT);
            }
            else if (col == COL_Text)
            {
                fl_color(text_color());
                fl_draw(listing_->line(row).c_str(), x, y, width, height, FL_ALIGN_LEFT);
            }
        }
        fl_pop_clip();
        break;

    default:
        break;
    }
}


void Fl_SourceTable::resize(int x, int y, int w, int h)
{
    Fl_Table::resize(x, y, w, h);
    int maxWidth = maxTextLen_;
    int textWidth = std::max(
        maxWidth,
        w - col_width(COL_Mark) - col_width(COL_Line) - 25);
    col_width(COL_Text, textWidth);
}


////////////////////////////////////////////////////////////////
//
// (Dis)Assembly Code Table
//
Fl_AsmTable::Fl_AsmTable(int x, int y, int w, int h, const char* label)
    : Fl_CodeTable(x, y, w, h, label)
    , listing_(nullptr)
    , rowNum_(-1)
    , maxAsmLen_(0)
{
    col_header(true);
    cols(4);

    col_width(COL_Mark, 30);
    col_width(COL_Addr, 160);
    col_width(COL_Text, 300);
    col_width(COL_Asm,  1000);
}


void Fl_AsmTable::draw_cell(
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
        fl_font(font(), font_size());
        break;

    case CONTEXT_COL_HEADER:
        break;

    case CONTEXT_CELL:
        if (row != rowNum_)
        {
            const string& line = listing_->line(row);
            boost::split(rowData_, line, boost::algorithm::is_any_of("\t"));
        }

        fl_push_clip(x, y, width, height);
        fl_color(cell_background(row, col));
        fl_rectf(x, y, width, height);
        
        fl_color(text_color());

        switch (col)
        {
        case COL_Mark:
            draw_line_marks(row, x, y);
            break;

        case COL_Addr:
            if (rowData_.size())
            {
                fl_draw(rowData_[0].c_str(), x, y, width, height, FL_ALIGN_LEFT);
            }
            break;
            
        case COL_Text:
            if (rowData_.size() > 1)
            {
                fl_draw(rowData_[1].c_str(), x, y, width, height, FL_ALIGN_LEFT);
            }
            break;

        case COL_Asm:
            if (rowData_.size() > 2)
            {
                const string& asmText = rowData_[2];
                if (strncmp(asmText.c_str(), "invalid", 7) == 0)
                {
                    break;
                }               
                if (asmText.length() > maxAsmLen_)
                {
                    maxAsmLen_ = asmText.length();
                }
                fl_draw(asmText.c_str(), x, y, width, height, FL_ALIGN_LEFT);
            }
        }
        fl_pop_clip();
        break;

    default:
        break;
    }
}


void Fl_AsmTable::refresh(

    const RefPtr<Thread>& t,
    const RefPtr<Symbol>& sym)

{
    if (listing_->refresh(t, sym))
    {
        maxAsmLen_ = 0;
    }

    int line = listing_->addr_to_line(sym->addr());
    set_mark_at_line(highlighted_line(), mark_arrow, false);
    highlight_line(listing_, line);
    set_mark_at_line(line, mark_arrow);
}


void Fl_AsmTable::resize(int x, int y, int w, int h)
{
    Fl_Table::resize(x, y, w, h);

    int maxWidth = maxAsmLen_;
    int asmWidth = std::max(
        maxWidth,
        w - col_width(COL_Mark) - col_width(COL_Addr) - col_width(COL_Text) - 25);
    col_width(COL_Asm, asmWidth);
}

