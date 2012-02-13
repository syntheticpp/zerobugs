#ifndef FLMENU_H__93D60C08_34D0_4D9C_98D1_19BBD50428EA
#define FLMENU_H__93D60C08_34D0_4D9C_98D1_19BBD50428EA
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "menu.h"
#include <FL/Fl_Menu_Bar.H>


class FlMenuBar : public ui::CompositeMenu
{
public:
    FlMenuBar(ui::Controller&, int w, int h);
    ~FlMenuBar() throw();

    virtual void add(RefPtr<ui::Menu>);

private:
    void exec_command(const char* path);

    static void exec_command(Fl_Widget*, void*);

private:
    ui::Controller& controller_;
    Fl_Menu_Bar*    menu_;
};


#endif // FLMENU_H__93D60C08_34D0_4D9C_98D1_19BBD50428EA
