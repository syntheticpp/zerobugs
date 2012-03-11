#ifndef FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E
#define FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/Fl_Table_Row.H>


class FlBreakPointTable : public Fl_Table_Row
{
public:
    FlBreakPointTable(int x, int y, int w, int h, const char* label);

protected:
    void draw_cell(TableContext, int row, int col, int x, int y, int w, int h);
};


#endif // FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E

