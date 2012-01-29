#ifndef FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67
#define FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: pack_layout.h 67464 2012-01-28 04:14:52Z unknown $
//
#include "view.h"
#include <FL/Fl_Pack.H>


namespace ui
{
    class FlPackLayout : public Layout
    {
    public:
        FlPackLayout();
        ~FlPackLayout();

        Fl_Group* get_group() { return group_; }

        virtual void add(View&);
        virtual void show(View&, bool);

    private:
        Fl_Group*   group_;
    };
}

#endif // FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67
