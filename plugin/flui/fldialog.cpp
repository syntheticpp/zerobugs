//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fldialog.h"
#include "controller.h"
#include <FL/Fl_Double_Window.H>
#include <iostream>
using namespace std;


FlDialog::FlDialog(

    ui::Controller& controller,
    int             x,
    int             y,
    int             w,
    int             h,
    const char* title /* = nullptr */
    )

    : ui::Dialog(controller)
    , window_(new Fl_Double_Window(x, y, w, h, title))
{
    window_->callback(close_callback, this);
}


FlDialog::~FlDialog()
{
#ifdef DEBUG
    clog << __PRETTY_FUNCTION__ << endl;
#endif
}


void FlDialog::center()
{
    int x = (controller().w() - window_->w()) / 2;
    int y = (controller().h() - window_->h()) / 2;

    window_->position(controller().x() + x, controller().y() + y);
}


void FlDialog::close_callback(Fl_Widget* w, void* data)
{
    reinterpret_cast<FlDialog*>(data)->close();
}


void FlDialog::set_resizable()
{
    window_->resizable(*window_);
}


void FlDialog::show(bool doShow)
{
    if (doShow)
    {
        window_->set_modal();
        window_->show();
    }
    else
    {
        window_->set_non_modal();
        window_->hide();
    }
}

