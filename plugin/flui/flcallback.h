#ifndef FLCALLBACK_H__113A14F9_46A5_4B59_9BE9_FBBF5A70BA2E
#define FLCALLBACK_H__113A14F9_46A5_4B59_9BE9_FBBF5A70BA2E
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "view.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Widget.H>


/**
 * "Stock" layout callback.
 */
class Callback : public ui::LayoutCallback
{
public:
    explicit Callback(Fl_Group* g, int x = 0, int y = 0, int w = 0, int h = 0)
        : group_(g)
        , widget_(nullptr)
        , x_(x)
        , y_(y)
        , w_(w)
        , h_(h)
    { }

    Callback(Fl_Group* g, Fl_Widget* w, int dx = 0, int dy = 0, int dw = 0, int dh = 0)
        : group_(g)
        , widget_(nullptr)
        , x_(w->x() + dx)
        , y_(w->y() + dy)
        , w_(w->w() + dw)
        , h_(w->h() + dh)
    { }

    void set_widget(Fl_Widget* w) {
        widget_ = w;
    }

    virtual void insert() {
        assert(group_);
        assert(widget_);

        widget_->resize(x_, y_, w_, h_);
        group_->add_resizable(*widget_);
    }

protected:
    Fl_Group* group() {
        return group_;
    }

    Fl_Widget* widget() {
        return widget_;
    }

private:
    Fl_Group*   group_;
    Fl_Widget*  widget_;
    int         x_, y_, w_, h_;
};

#endif // FLCALLBACK_H__113A14F9_46A5_4B59_9BE9_FBBF5A70BA2E

