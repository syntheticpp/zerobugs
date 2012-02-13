//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fltoolbar.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Pixmap.H>


FlToolbar::FlToolbar(

    ui::Controller& c,
    int             width,
    int             height )

    : base_type(c, 0, 25, width, height)
{
    widget()->type(Fl_Pack::HORIZONTAL);
    widget()->box( FL_UP_FRAME );
    widget()->spacing(4);
    widget()->end();
}


void FlToolbar::add_button(
    const char*         tooltip, 
    const char* const*  pixmap,
    ui::CommandPtr      command)

{
    widget()->begin();

    Fl_Button* b = new Fl_Button(0, 0, 30, 30);
    b->box(FL_FLAT_BOX);
    b->clear_visible_focus();
    b->tooltip(tooltip);
    b->image(new Fl_Pixmap(pixmap));

    widget()->end();
}

