#ifndef FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67
#define FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: flpack_layout.h 67464 2012-01-28 04:14:52Z unknown $
//
#include "code_view.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>


class FlPackLayout : public ui::Layout
{
    ~FlPackLayout() throw();

public:
    FlPackLayout(ui::Controller&, int x, int y, int w, int h);

    //Fl_Group* group() { return group_; }

    // View interface
    virtual void added_to(const ui::View&);
    virtual void update(const ui::State&);

    // Layout interface
    virtual void add(ui::View&);
    virtual void show(ui::View&, bool);

    void add_code_view(Fl_Widget* w);

protected:
    int code_height() const;

private:
    Fl_Group*   group_; // this group
    Fl_Group*   codeArea_;
    Fl_Tabs*    bottom_;
    Fl_Tabs*    right_;
};


#endif // FLPACK_LAYOUT_H__C9EE8D01_7D07_4F48_9578_2355F6269D67
