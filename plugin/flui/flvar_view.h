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
 * Display program variables using a Fl_Table widget.
 */
class Fl_VarTable : public Fl_Table
{
public:
    static const int COL_VarName     = 0;
    static const int COL_VarValue    = 1;
    static const int COL_VarType     = 2;

    explicit Fl_VarTable(
        ui::VarView*    view,
        int             x,
        int             y,
        int             w,
        int             h,
        const char*     label);

private:
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
    
    void draw_symbol(
        int             row,
        int             col,
        int             x,
        int             y,
        int             w,
        int             h);

    virtual void resize(int x, int y, int w, int h);

    void event_callback();

    static void event_callback(Fl_Widget*, void*);

private:
    ui::VarView* view_;
};



class FlVarView : public FlView<ui::VarView, Fl_VarTable>
{
public:
    explicit FlVarView(ui::Controller&);

    virtual void update(const ui::State&);
};



/**
 * View variables that local to a scope.
 */
class FlLocalsView : public FlView<ui::LocalsView, Fl_VarTable>
{
public:
    explicit FlLocalsView(ui::Controller&, const char* = "Locals");

    virtual void update(const ui::State&);
};

#endif // FLVAR_VIEW_H__CE09CF79_2049_42E6_BD43_6EF8BFCB1DEC

