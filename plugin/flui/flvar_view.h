#ifndef FLVAR_VIEW_H__CE09CF79_2049_42E6_BD43_6EF8BFCB1DEC
#define FLVAR_VIEW_H__CE09CF79_2049_42E6_BD43_6EF8BFCB1DEC
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/Fl_Table.H>


/**
 * View program variables using a Fl_Table widget.
 */
class FlVarView : public Fl_Table
{
public:
    FlVarView(int x, int y, int w, int h, const char* label = nullptr);

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


#endif // FLVAR_VIEW_H__CE09CF79_2049_42E6_BD43_6EF8BFCB1DEC

