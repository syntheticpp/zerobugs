#ifndef FLTOOLBAR_H__2600BF29_75DA_445E_B7B1_81D04B8ED47B
#define FLTOOLBAR_H__2600BF29_75DA_445E_B7B1_81D04B8ED47B
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flview.h"
#include "toolbar.h"
#include <FL/Fl_Pack.H>


class FlToolbar : public FlView<ui::Toolbar, Fl_Pack>
{
public:
    FlToolbar(ui::Controller& c, int width, int height);

protected:
    void add_button(
        const char*         tooltip,
        const char* const*  pixmap,
        ui::CommandPtr      command);
};


#endif // FLTOOLBAR_H__2600BF29_75DA_445E_B7B1_81D04B8ED47B

