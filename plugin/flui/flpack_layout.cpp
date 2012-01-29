//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flpack_layout.h"
#include <FL/Fl_Pack.H>
#include <FL/Fl_Tile.H>

#include <FL/Fl_Box.H>


FlPackLayout::FlPackLayout(int x, int y, int w, int h)
    : group_(new Fl_Tile(x, y + 30, w, h - 30))
{
    group_->begin();
    {
        // place-holders
        Fl_Tile* tile = new Fl_Tile(0, y + 30, w, 400);
            tile->type(Fl_Pack::VERTICAL);
            Fl_Box* b = new Fl_Box(0, y + 30, w - 250, 400, "Area One");
            b->box(FL_DOWN_BOX);
            b = new Fl_Box(w - 250, y + 30, 250, 400, "Area Two");
            b->box(FL_DOWN_BOX);
        tile->box(FL_DOWN_BOX);
        tile->end();
    }

    {
        Fl_Box* b = new Fl_Box(0, y + 430, w, h - 430, "Area Three");
        b->box(FL_DOWN_BOX);
    }
    group_->end();
}


FlPackLayout::~FlPackLayout()
{
}


void FlPackLayout::update(const ui::State&)
{
}

void FlPackLayout::accept(ui::Layout&)
{
}

void FlPackLayout::add(ui::View&)
{
}


void FlPackLayout::show(ui::View& view, bool show)
{
}


