#ifndef FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E
#define FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Table_Row.H>

namespace ui
{
    class BreakPointView;
}


/**
 * Draw a table of breakpoints. Used by FlBreakPointView.
 */
class FlBreakPointTable : public Fl_Table_Row
{
    typedef ui::BreakPointView View;
    typedef std::unique_ptr<Fl_Pixmap> PixmapPtr;

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

    PixmapPtr           checked_;
    PixmapPtr           unchecked_;
};


#endif // FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E

