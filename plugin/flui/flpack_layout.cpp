//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "const.h"
#include "flpack_layout.h"
#include "zdk/thread_util.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Tile.H>
#include <iostream>

using namespace ui;
using namespace std;


FlPackLayout::FlPackLayout(ui::Controller& c, int x, int y, int w, int h)
    : ui::Layout(c)
    , group_(new Fl_Tile(x, Const::layout_y(y), w, Const::layout_height(h)))
    , code_(nullptr)
    , bottomL_(nullptr)
    , bottomR_(nullptr)
    , right_(nullptr)
    , status_(nullptr)
{
    group_ = new Fl_Tile(x, Const::layout_y(y), w, Const::layout_height(h));
    Fl_Tile* vtile = new Fl_Tile(group_->x(), Const::layout_y(y), w, code_height());
    vtile->type(Fl_Pack::VERTICAL);
    vtile->box(FL_DOWN_BOX);

    // area where source or assembly code is displayed
    code_ = new Fl_Group(vtile->x(), Const::layout_y(y),
        vtile->w() - Const::thread_regs_width, code_height());
    code_->box(FL_DOWN_BOX);
    code_->end();

    // area where Threads and Registers are displayed,
    // wrapped inside a group widget
    auto g = new Fl_Group(
        vtile->x() + code_->w(),
        Const::layout_y(y),
        Const::thread_regs_width,
        vtile->h());
    g->box(FL_DOWN_BOX);

    right_ = new Fl_Tabs(
        g->x() + 1,
        g->y() + 1,
        Const::thread_regs_width - 2,
        code_height() - 2);
    ////////////////////////////////////////////////////////////
    //
    // place holders
    //
    {
        auto b = new Fl_Box(right_->x(), right_->y(), right_->w(),
            right_->h() - Const::tab_label_height, "Threads");
        b->labelfont(FL_HELVETICA);
        b = new Fl_Box(right_->x(), right_->y(), right_->w(),
            right_->h() - Const::tab_label_height, "Registers");
        b->labelfont(FL_HELVETICA);

        right_->end();
    }
    g->end();
    vtile->end();

    ////////////////////////////////////////////////////////////
    //
    // area for stack traces, local variables and watches
    //
    g = new Fl_Group(
        group_->x(),
        y + code_height() + Const::menubar_height,
        group_->w(),
        h - code_height() - Const::menubar_height - Const::statbar_height);

    g->box(FL_DOWN_BOX);
    {   // horizontal tile contains a left-side tabs
        // and a right-side tabs widget
        Fl_Tile* tile = new Fl_Tile(g->x() + 2, g->y(), g->w() - 4, g->h());
        tile->type(Fl_Pack::HORIZONTAL);
        tile->box(FL_DOWN_BOX);
        auto flat = new Fl_Group(tile->x(), tile->y(), tile->w() / 2, tile->h());
        flat->box(FL_FLAT_BOX);
        bottomL_ = new Fl_Tabs(tile->x(), tile->y(), tile->w() / 2, tile->h());
        bottomL_->end();
        flat->end();

        flat = new Fl_Group(
            tile->x() + tile->w() / 2, tile->y(),
            tile->w() - tile->w() / 2, tile->h());
        flat->box(FL_FLAT_BOX);

        bottomR_ = new Fl_Tabs(
            tile->x() + tile->w() / 2, tile->y(),
            tile->w() - tile->w() / 2, tile->h());
        bottomR_->end();
        flat->end();
        tile->end();
    }
    g->end();
    group_->end();

    // status bar
    status_ = new Fl_Output(
        x + 2,
        y + group_->y() + group_->h() + 1,
        w - 4,
        Const::statbar_height - 2);
    status_->textfont(FL_SCREEN);
    status_->textsize(12);
    status_->color(FL_LIGHT1);
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

    if (status_ && s.current_event() != E_PROMPT)
    {
        update_status(s);
    }
    group_->redraw();
}


void FlPackLayout::update_status(const ui::State& s)
{
    assert(status_);

    if (s.current_thread())
    {
        if (auto descr = thread_get_event_description(*s.current_thread()))
        {
            status_->value(descr->c_str(), descr->length());
        }
    }
}


void FlPackLayout::show(ui::View& view, bool show)
{
    // TODO
}


void FlPackLayout::status_message(const std::string& msg)
{
    if (status_)
    {
        status_->value(msg.c_str(), msg.length());
    }
}

