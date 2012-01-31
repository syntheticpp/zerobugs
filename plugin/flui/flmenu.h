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
    FlMenuBar(ui::Controller&, Fl_Window*);
    ~FlMenuBar() throw();

    virtual void add(RefPtr<ui::Menu>);

private:
    static void exec_menu_item(Fl_Widget*, void*);

    ui::Controller& controller_;
    Fl_Menu_Bar*    menu_;
};


#endif // FLMENU_H__93D60C08_34D0_4D9C_98D1_19BBD50428EA
