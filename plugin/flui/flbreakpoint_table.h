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
class Fl_BreakPointTable : public Fl_Table_Row
{
public:
    typedef ui::BreakPointView View;
    typedef std::unique_ptr<Fl_Pixmap> PixmapPtr;
    typedef std::function<void ()> EventCallback;

    enum ColType
    {
        COL_Pixmap,
        COL_File,
        COL_Line,
        COL_Address,
        COL_Condition
    };

    Fl_BreakPointTable(
        View*       v,
        int         x,
        int         y,
        int         w,
        int         h,
        const char* label);

    template<typename F>
    void set_event_callback(F f) {
        eventCallback_ = f;
    }

protected:
    virtual void draw_cell(
        TableContext ctxt,
        int          row,
        int          col,
        int          x,
        int          y,
        int          w,
        int          h);

    static void event_callback(Fl_Widget*, void*);

    void event_callback();

private:
    ui::BreakPointView* view_;

    PixmapPtr           checked_;
    PixmapPtr           unchecked_;
    EventCallback       eventCallback_;
};


#endif // FLBREAKPOINT_TABLE_H__B4876CB4_0BBE_487F_AB90_86304AEFBB8E

