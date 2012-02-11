//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/stdexcept.h"
#include "flcode_table.h"
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <cassert>
#include <iostream>
#include <fstream>

using namespace std;

enum ColumnType
{
    COL_MARK,
    COL_LINE,
    COL_TEXT
};


Fl_CodeTable::Fl_CodeTable(int x, int y, int w, int h, const char* label)
    : Fl_Table(x, y, w, h, label)
    , font_(FL_SCREEN)  // Terminal style font
    , fontSize_(11)
    , highlight_(0)
    , maxWidth_(0)
    , digits_(0)
{
    col_header(true);

    cols(3);// one column for markers, one for line numbers, third for text

    col_width(COL_MARK, 30);
    col_width(COL_LINE, 50);
    col_width(COL_TEXT, 500);
}


int Fl_CodeTable::line_digits() const
{
    if (digits_ == 0)
    {
        size_t n = lines_.size();
        while (n)
        {
            n /= 10;
            ++digits_;
        }
    }
    return digits_;
}


void Fl_CodeTable::draw_header(int col, int x, int y, int w, int h)
{
    fl_push_clip(x, y, w, h);

    fl_color(header_background());
    fl_rectf(x, y, w, h);

    if (col == COL_TEXT)
    {
        fl_color(text_color());
        fl_draw(filename_.c_str(), x, y, w, h, FL_ALIGN_LEFT);
    }

    fl_pop_clip();
}


void Fl_CodeTable::draw_line_marks(int row, int x, int y)
{
    // draw pixmap marks if any
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


void Fl_CodeTable::draw_cell(
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
        fl_font(font_, fontSize_);
        break;

    case CONTEXT_COL_HEADER:
        draw_header(col, x, y, width, height);
        break;

    case CONTEXT_CELL:
        fl_push_clip(x, y, width, height);
        {
            if (col == COL_MARK)
            {
                fl_color(FL_LIGHT2);
            }
            else
            {
                fl_color(row + 1 == highlight_ ? highlight_background() : text_background());
            }
            fl_rectf(x, y, width, height);

            if (col == COL_MARK)
            {
                draw_line_marks(row, x, y);
            }
            else if (col == COL_LINE)
            {
                char s[16] = { 0 };

                snprintf(s, sizeof(s - 1), "%*d", line_digits(), row + 1);

                fl_color(text_color());
                fl_draw(s, x, y, width, height, FL_ALIGN_LEFT);
            }
            else if (col == COL_TEXT)
            {
                fl_color(text_color());
                fl_draw(lines_[row].c_str(), x, y, width, height, FL_ALIGN_LEFT);
            }
        }
        fl_pop_clip();
        break;

    default:
        break;
    }
}


void Fl_CodeTable::read_file(const char* filename)
{
    if (filename_ == filename)
    {
        return;
    }
    lines_.clear();

    highlight_ = 0;
    maxWidth_ = 0;
    digits_ = 0;

    // if source code lines exceed this size, though luck - truncate them
    vector<char> buf(2048);

    ifstream fin(filename);
    if (fin)
    {
        filename_ = filename;

        while (fin.getline(&buf[0], buf.size()))
        {
            lines_.push_back(&buf[0]);
            if (lines_.back().length() > maxWidth_)
            {
                maxWidth_ = lines_.back().length();
            }
        }
        rows(lines_.size());
    }
}


void Fl_CodeTable::highlight_line(int line)
{
    highlight_ = line;
    --line;

    if (line < 0 || size_t(line) >= lines_.size())
    {
        throw out_of_range(__func__);
    }
    row_position(line);
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


void Fl_CodeTable::resize(int x, int y, int w, int h)
{
    Fl_Table::resize(x, y, w, h);

    int textWidth = std::max(maxWidth_, w - col_width(COL_MARK) - col_width(COL_LINE) - 25);
    col_width(COL_TEXT, textWidth);
}

