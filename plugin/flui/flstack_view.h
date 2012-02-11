#ifndef FLSTACK_VIEW_H__C0614364_4BF5_45B7_96ED_09348EFB0991
#define FLSTACK_VIEW_H__C0614364_4BF5_45B7_96ED_09348EFB0991
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flview.h"
#include "stack_view.h"
#include <FL/Fl_Table_Row.H>


class Fl_StackTable : public Fl_Table_Row
{
public:
    Fl_StackTable(ui::StackView*, int x, int y, int w, int h, const char*);

private:
    virtual void resize(int x, int y, int w, int h);

    /** 
     * implements custom drawing
     * @see http://seriss.com/people/erco/Fl_Table/documentation/Fl_Table.html#draw_cell
     */
    virtual void draw_cell(
        TableContext    ctxt,
        int             row,
        int             col,
        int             x,
        int             y,
        int             w,
        int             h);
    
    void draw_frame(
        int             row,
        int             col,
        int             x,
        int             y,
        int             w,
        int             h);

    void event_callback();

    static void event_callback(Fl_Widget*, void*);

private:
    ui::StackView*  view_;
};


class FlStackView : public FlView<ui::StackView, Fl_StackTable>
{
public:
    explicit FlStackView(ui::Controller&);

    void update(const ui::State&);
};

#endif // FLSTACK_VIEW_H__C0614364_4BF5_45B7_96ED_09348EFB0991

