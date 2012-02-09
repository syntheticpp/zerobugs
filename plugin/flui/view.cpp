//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"


ui::View::View(ui::Controller& c) : c_(c)
{
}


ui::Layout::Layout(ui::Controller& c) : View(c)
{
}


void ui::Layout::add(Callback& cb, ui::View& v)
{
    views_.push_back(&v);
    cb.insert(v);
}


void ui::Layout::update(const ui::State& s)
{
    for (auto v : views_)
    {
        v->update(s);
    }
}

