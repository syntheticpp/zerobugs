//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "flsplitter.h"
#include <assert.h>
#include <FL/Fl.H>
#include <iostream>

using namespace std;


void FlSplitter::resize(int x, int y, int w, int h)
{
    Fl_Tile::resize(x, y, w, h);
    enforce_vertical() || enforce_horizontal();
}


int FlSplitter::handle(int eventType)
{
    switch (eventType)
    {
    case FL_DRAG:
        {
            assert(children() == 2);
            Fl_Widget* c1 = child(0);
            Fl_Widget* c2 = child(1);
            if (type() == VERTICAL)
            {
                if (c1->h() < minSize_ || c2->h() < minSize_)
                {
                    return 1;
                }
            }
            else if (type() == HORIZONTAL)
            {
                if (c1->w() < minSize_ || c2->w() < minSize_)
                {
                    return 1;
                }
            }
        }
        break;

    case FL_RELEASE:
        if (enforce_vertical() || enforce_horizontal())
        {
            redraw();
            return 1;
        }
        break;
    }

    return Fl_Tile::handle(eventType);
}


bool FlSplitter::enforce_horizontal()
{
    if (type() != HORIZONTAL)
    {
        return false;
    }
    Fl_Widget* c1 = child(0);
    Fl_Widget* c2 = child(1);

    if (c1->w() < minSize_)
    {
        int d = c1->w() - minSize_;
        c1->resize(c1->x(), c1->y(), minSize_, c1->h());
        c2->resize(c2->x() - d, c2->y(), c2->w() + d, c2->h());
        return true;
    }
    if (c2->w() < minSize_)
    {
        int d = c2->w() - minSize_;
        c1->resize(c1->x(), c1->y(), c1->w() + d, c1->h());
        c2->resize(c2->x() + d, c2->y(), minSize_, c2->h());
        return true;
    }
    return false;
}


bool FlSplitter::enforce_vertical()
{
    if (type() != VERTICAL)
    {
        return false;
    }
    Fl_Widget* c1 = child(0);
    Fl_Widget* c2 = child(1);

    if (c1->h() < minSize_)
    {
        int d = c1->h() - minSize_;
        c1->resize(c1->x(), c1->y(), c1->w(), minSize_);
        c2->resize(c2->x(), c2->y() - d, c2->w(), c2->h() + d);
        return true;
    }
    if (c2->h() < minSize_)
    {
        int d = c2->h() - minSize_;
        c1->resize(c1->x(), c1->y(), c1->w(), c1->h() + d);
        c2->resize(c2->x(), c2->y() + d, c2->w(), minSize_);
        return true;
    }
    return false;
}
