#ifndef FLSPLITTER_H__8DAAD4C5_446D_4F4F_97F2_3F55053A15E1
#define FLSPLITTER_H__8DAAD4C5_446D_4F4F_97F2_3F55053A15E1
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include <FL/Fl_Tile.H>


class FlSplitter : public Fl_Tile
{
public:
    enum Type {
        VERTICAL = 0,
        HORIZONTAL = 1
    };

    FlSplitter(int x, int y, int w, int h)
        : Fl_Tile(x, y, w, h)
        , minSize_(150) {
    }

    void set_min_size(int size) {
        minSize_ = size;
    }

    bool enforce_horizontal();
    bool enforce_vertical();

    void resize(int x, int y, int w, int h);

private:
    int handle(int);

private:
    int minSize_;
};

#endif // FLSPLITTER_H__8DAAD4C5_446D_4F4F_97F2_3F55053A15E1

