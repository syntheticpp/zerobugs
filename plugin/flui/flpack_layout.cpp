//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flpack_layout.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>



// todo: move these to a header file and include in flmenu.cpp
static const int MENUBAR_HEIGHT     = 25 + 30;
static const int STATBAR_HEIGHT     = 25;
static const int THREAD_REGS_WIDTH  = 250;


//todo: change name to FlTileLayout

FlPackLayout::FlPackLayout(ui::Controller& c, int x, int y, int w, int h)
    : ui::Layout(c)
    , group_(new Fl_Tile(x, y + MENUBAR_HEIGHT, w, h - MENUBAR_HEIGHT - STATBAR_HEIGHT))
    , code_(nullptr)
    , bottom_(nullptr)
    , right_(nullptr)
{
    Fl_Tile* tile = new Fl_Tile(0, y + MENUBAR_HEIGHT, w, code_height());
    tile->type(Fl_Pack::VERTICAL);
    tile->box(FL_DOWN_BOX);

    // area where source or assembly code is displayed
    auto pack = new Fl_Group(x, y + MENUBAR_HEIGHT, w - THREAD_REGS_WIDTH, code_height());

    code_ = pack;
    code_->box(FL_DOWN_BOX);
    code_->end();

    // area where Threads and Registers are displayed
    auto g = new Fl_Group(x + code_->w(), y + MENUBAR_HEIGHT, THREAD_REGS_WIDTH, tile->h());
    g->box(FL_DOWN_BOX);
    right_ = new Fl_Tabs(g->x() + 2, g->y() + 2, 246, code_height() - 4);

    // place holders
    auto b = new Fl_Box(right_->x(), right_->y(), right_->w(), right_->h() - 22, "Threads");
    b->labelfont(FL_HELVETICA);
    b = new Fl_Box(right_->x(), right_->y(), right_->w(), right_->h() - 22, "Registers");
    b->labelfont(FL_HELVETICA);

    right_->end();
    g->end();

    tile->end();

    // area for stack traces, local variables and watches
    g = new Fl_Group(x, y + code_height() + MENUBAR_HEIGHT, w, h - code_height() - MENUBAR_HEIGHT - STATBAR_HEIGHT);
    g->box(FL_DOWN_BOX);
    bottom_ = new Fl_Tabs(g->x() + 2, g->y() + 2, g->w() - 4, g->h() - 4);
    bottom_->end();
    g->end();
    group_->end();

    auto status = new Fl_Box(x, y + group_->y() + group_->h(), w - 2, STATBAR_HEIGHT - 2 /* , "Status Bar" */);
    status->box(FL_BORDER_BOX);
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
    group_->redraw();
}


void FlPackLayout::show(ui::View& view, bool show)
{
}

