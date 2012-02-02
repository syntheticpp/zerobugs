//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"

void ui::Layout::add(ui::View& v)
{
    views_.push_back(&v);
    v.added_to(*this);
}


void ui::Layout::update(const ui::State& s)
{
    for (auto v = views_.begin(); v != views_.end(); ++v)
    {
        (*v)->update(s);
    }
}

