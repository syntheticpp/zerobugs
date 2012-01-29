#ifndef FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67
#define FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: flpack_layout.h 67464 2012-01-28 04:14:52Z unknown $
//
#include "view.h"
#include <FL/Fl_Group.H>


class FlPackLayout : public ui::Layout
{
public:
    FlPackLayout(int x, int y, int w, int h);
    ~FlPackLayout();

    Fl_Group* group() { return group_; }

    // View intterface
    virtual void accept(ui::Layout&);
    virtual void update(const ui::State&);

    // Layout interface
    virtual void add(ui::View&);
    virtual void show(ui::View&, bool);

private:
    Fl_Group*   group_;
};


#endif // FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67