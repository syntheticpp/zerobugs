#ifndef FLTABBED_H__A46B24D2_E6FD_4FCA_834B_645A1E2D578C
#define FLTABBED_H__A46B24D2_E6FD_4FCA_834B_645A1E2D578C
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/Fl_Tabs.H>


class Fl_Tabbed : public Fl_Tabs
{
public:
    Fl_Tabbed(int x, int y, int w, int h)
        : Fl_Tabs(x, y, w, h)
    { }

    int handle(int event);
};

#endif // FLTABBED_H__A46B24D2_E6FD_4FCA_834B_645A1E2D578C

