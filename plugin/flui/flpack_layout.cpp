//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flcode_view.h"
#include "flcode_table.h"
#include "flpack_layout.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>

#include <FL/Fl_Box.H>
#include <iostream>


// todo: move these to a header file and include in flmenu.cpp
static const int menubar_h  = 30;
static const int statbar_h  = 30;

//todo: change name to FlTileLayout

FlPackLayout::FlPackLayout(ui::Controller& c, int x, int y, int w, int h)
    : ui::Layout(c)
    , group_(new Fl_Tile(x, y + menubar_h, w, h - menubar_h - statbar_h))
    , codeArea_(nullptr)
    , bottom_(nullptr)
    , right_(nullptr)
{
    Fl_Tile* tile = new Fl_Tile(0, y + menubar_h, w, code_height());
    tile->type(Fl_Pack::VERTICAL);
    tile->box(FL_DOWN_BOX);

    // area where source (or assembly) code is displayed
    auto pack = new Fl_Group(0, y + menubar_h, w - 250, code_height());

    codeArea_ = pack;
    codeArea_->box(FL_DOWN_BOX);
    codeArea_->end();
    {
        // place-holder for area where Threads and Registers are displayed
        //right_ = new Fl_Tabs(w - 250, y + menubar_h, 250, code_height());
        //auto b = new Fl_Box(w - 250, y + menubar_h, 250, code_height() - 30, "Area Two");
        //right_->end();

        auto b = new Fl_Box(w - 250, y + menubar_h, 250, code_height(), "Area Two");
        b->box(FL_DOWN_BOX);
        tile->end();
    }

    // area for stack traces, local variables and watches
    {
    #if 1
        bottom_ = new Fl_Tabs(0, y + code_height() + menubar_h, w, h - code_height() - menubar_h);
        auto b = new Fl_Box(0, y + code_height() + menubar_h, w, h - code_height() - menubar_h - 30, "Area Three");
        bottom_->end();
        bottom_->resizable(b);
    #else
        auto b = new Fl_Box(0, y + code_height() + menubar_h, w, h - code_height() - menubar_h, "Area Three");
    #endif
        b->box(FL_DOWN_BOX);
    }
    group_->end();
}


FlPackLayout::~FlPackLayout() throw()
{
}

int FlPackLayout::code_height() const
{
    return 500;
}

void FlPackLayout::update(const ui::State& s)
{
    ui::Layout::update(s);
}


void FlPackLayout::added_to(const ui::View&)
{
}


void FlPackLayout::add(ui::View& v)
{
    if (auto cv = dynamic_cast<FlMultiCodeView*>(&v))
    {
        add_code_view(cv->widget());
    }
    ui::Layout::add(v);
}


void FlPackLayout::add_code_view(Fl_Widget* w)
{
    assert(codeArea_);

    w->resize(codeArea_->x(), codeArea_->y(), codeArea_->w(), codeArea_->h());
    codeArea_->add_resizable(*w);
}


void FlPackLayout::show(ui::View& view, bool show)
{
}


