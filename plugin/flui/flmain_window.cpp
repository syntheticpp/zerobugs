//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "controller.h"
#include "flmain_window.h"
// #include <iostream>

using namespace std;


FlMainWindow::FlMainWindow(

    ui::Controller& c,
    int             x,
    int             y,
    int             w,
    int             h,
    const char*     label )

    : Fl_Double_Window(x, y, w, h, label)
    , controller_(c)
{
}


// just a debug hook for monitoring events
int FlMainWindow::handle(int eventType)
{
    return Fl_Double_Window::handle(eventType); 
}

