//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "utils.h"
#include <FL/Fl_Output.H>


Fl_Output* static_text (
    int         x,
    int         y,
    int         w,
    int         h,
    const char* value )

{
    auto staticText = new Fl_Output(x, y, w, h);
    if (value)
    {
        staticText->value(value);
    }
    staticText->box(FL_FLAT_BOX);
    staticText->color(FL_BACKGROUND_COLOR);
    staticText->clear_visible_focus();

    return staticText;
}


