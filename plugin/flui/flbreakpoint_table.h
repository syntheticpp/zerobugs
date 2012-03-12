#ifndef FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E
#define FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/Fl_Table_Row.H>

namespace ui
{
    class BreakPointView;
}


class FlBreakPointTable : public Fl_Table_Row
{
    typedef ui::BreakPointView View;
public:
    FlBreakPointTable(
        View*       v,
        int         x,
        int         y,
        int         w,
        int         h,
        const char* label);

protected:
    virtual void draw_cell(
        TableContext ctxt,
        int          row,
        int          col,
        int          x,
        int          y,
        int          w,
        int          h);

private:
    ui::BreakPointView* view_;
};


#endif // FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E

