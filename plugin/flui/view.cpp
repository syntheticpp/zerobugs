//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "controller.h"
#include "view.h"


ui::View::View(ui::Controller& c) : c_(c)
{
}


void ui::View::interrupt_main_thread()
{
    c_.interrupt_main_thread();
}


ui::Layout::Layout(ui::Controller& c) : View(c)
{
}


void ui::Layout::add(ui::View& v, ui::LayoutCallback& cb)
{
    views_.push_back(&v);
    v.insert_self(cb);
}


void ui::Layout::update(const ui::State& s)
{
    for (auto v : views_)
    {
        v->update(s);
    }
}

