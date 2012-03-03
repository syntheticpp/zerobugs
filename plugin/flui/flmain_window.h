#ifndef FLMAIN_WINDOW_H__EAFFF12F_3A34_4E56_A99B_78FEE8EAC8DE
#define FLMAIN_WINDOW_H__EAFFF12F_3A34_4E56_A99B_78FEE8EAC8DE
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include <FL/Fl_Double_Window.H>

namespace ui
{
    class Controller;
}


class FlMainWindow : public Fl_Double_Window
{
public:

    FlMainWindow(
        ui::Controller&,
        int x,
        int y,
        int w,
        int h,
        const char* title = nullptr);

private:
    // virtual int handle(int);

private:
    ui::Controller& controller_;
};

#endif // FLMAIN_WINDOW_H__EAFFF12F_3A34_4E56_A99B_78FEE8EAC8DE

