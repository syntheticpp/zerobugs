//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "const.h"
#include "flpack_layout.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>

using namespace ui;


//todo: change name to FlTileLayout

FlPackLayout::FlPackLayout(ui::Controller& c, int x, int y, int w, int h)
    : ui::Layout(c)
    , group_(new Fl_Tile(x, Const::layout_y(y), w, Const::layout_height(h)))
    , code_(nullptr)
    , bottomL_(nullptr)
    , bottomR_(nullptr)
    , right_(nullptr)
{
    Fl_Tile* tile = new Fl_Tile(0, Const::layout_y(y), w, code_height());
    tile->type(Fl_Pack::VERTICAL);
    tile->box(FL_DOWN_BOX);

    // area where source or assembly code is displayed
    auto pack = new Fl_Group(x, Const::layout_y(y), w - Const::thread_regs_width, code_height());

    code_ = pack;
    code_->box(FL_DOWN_BOX);
    code_->end();

    // area where Threads and Registers are displayed
    auto g = new Fl_Group(x + code_->w(), Const::layout_y(y), Const::thread_regs_width, tile->h());
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
    g = new Fl_Group(
        x,
        y + code_height() + Const::menubar_height,
        w,
        h - code_height() - Const::menubar_height - Const::statbar_height);
    g->box(FL_DOWN_BOX);
    {
        Fl_Tile* tile = new Fl_Tile(g->x() + 2, g->y() + 2, g->w() - 4, g->h() - 4);
        tile->type(Fl_Pack::HORIZONTAL);
        tile->box(FL_DOWN_BOX);
        auto g1 = new Fl_Group(g->x() + 2, g->y() + 2, g->w() / 2, g->h() - 4);
        g1->box(FL_DOWN_BOX);
        bottomL_ = new Fl_Tabs(g->x() + 2, g->y() + 2, g->w() / 2, g->h() - 4);
        bottomL_->end();
        g1->end();
        auto g2  = new Fl_Group(g->x() + 2 + g->w() / 2, g->y() + 2, g->w() / 2 - 4, g->h() - 4);
        bottomR_ = new Fl_Tabs(g->x() + 2 + g->w() / 2, g->y() + 2, g->w() / 2 - 4, g->h() - 4);
        bottomR_->end();
        g2->end();
        tile->end();
    }
    g->end();
    group_->end();

    auto status = new Fl_Box(x, y + group_->y() + group_->h(), w - 2, Const::statbar_height - 2);
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

