#ifndef FLVAR_VIEW_H__CE09CF79_2049_42E6_BD43_6EF8BFCB1DEC
#define FLVAR_VIEW_H__CE09CF79_2049_42E6_BD43_6EF8BFCB1DEC
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flview.h"
#include "locals_view.h"
#include <FL/Fl_Table.H>


/**
 * View program variables using a Fl_Table widget.
 */
class Fl_VarTable : public Fl_Table
{
public:
    explicit Fl_VarTable(int x = 0, int y = 0, int w = 0, int h = 0, const char* label = nullptr);

private:
    /** 
     * implements custom drawing
     * @see http://seriss.com/people/erco/Fl_Table/documentation/Fl_Table.html#draw_cell
     */
    virtual void draw_cell(
        TableContext,
        int row,
        int col,
        int x,  
        int y,  
        int w,  
        int h);
};



class FlLocalsView : public FlView<ui::LocalsView, Fl_VarTable>
{
public:
    explicit FlLocalsView(ui::Controller& c) : base_type(c, 0, 0, 0, 0, "Locals")
    {
    }
};

#endif // FLVAR_VIEW_H__CE09CF79_2049_42E6_BD43_6EF8BFCB1DEC

