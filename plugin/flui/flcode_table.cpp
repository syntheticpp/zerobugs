//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flcode_table.h"
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <cassert>
#include <iostream>
#include <fstream>

using namespace std;

FlCodeTable::FlCodeTable(int x, int y, int w, int h, const char* label)
    : Fl_Table(x, y, w, h, label)
{
    col_header(true);

    cols(3);// one column for markers, one for line numbers, third for text
            // todo: use row headers for line numbers?
    col_width(0, 30);
    col_width(1, 50);
    col_width(2, 1000);
}


// hacked example from:
// http://seriss.com/people/erco/Fl_Table/documentation/Fl_Table.html#draw_cell
void FlCodeTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
{
    switch (context)
    {
    case CONTEXT_STARTPAGE:
        fl_font(FL_COURIER, 11);         // todo: configurable font
        break;

    case CONTEXT_COL_HEADER:
        fl_push_clip(X, Y, W, H);
        {
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, color());
            fl_color(FL_BLACK);
            if (C == 2)
            {
                fl_draw(filename_.c_str(), X, Y, W, H, FL_ALIGN_LEFT);
            }
        }
        fl_pop_clip();
        break;

    case CONTEXT_CELL:
        fl_push_clip(X, Y, W, H);
        {
            char s[16] = { 0 };

            // BG COLOR
            if (C == 0)
            {
                fl_color(FL_LIGHT2);
            }
            else
            {
                fl_color( /*row_selected(R) ? selection_color() : */ FL_WHITE);
            }
            fl_rectf(X, Y, W, H);

            // TEXT

            if (C == 0)
            {
                //todo: draw markers
            }
            else if (C == 1)
            {
                sprintf(s, "%05d", R + 1);
                fl_color(FL_BLUE);
                fl_draw(s, X, Y, W, H, FL_ALIGN_LEFT);
            }
            else if (C == 2)
            {
                fl_color(FL_BLACK);
                fl_draw(lines_[R].c_str(), X, Y, W, H, FL_ALIGN_LEFT);
            }
        }
        fl_pop_clip();
        break;

    case CONTEXT_ENDPAGE:
    default:
        break;
    }
}


void FlCodeTable::read_file(const char* filename)
{
    // if source code lines exceed this size, though luck - truncate tehm
    vector<char> buf(2048);

    ifstream fin(filename);
    if (fin)
    {
        filename_ = filename;

        while (fin.getline(&buf[0], buf.size()))
        {
            lines_.push_back(&buf[0]);
        }

        rows(lines_.size());
    }
}

