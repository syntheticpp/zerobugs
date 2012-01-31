//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "flmenu.h"
#include <iostream>
#include <typeinfo>
#include <FL/Fl_Window.H>


FlMenuBar::FlMenuBar(

    ui::Controller& c,
    Fl_Window*      w

) : ui::CompositeMenu(c)
  , controller_(c)
  , menu_(new Fl_Menu_Bar(0, 0, w->w(), 30))
{
}


FlMenuBar::~FlMenuBar() throw()
{
}


void FlMenuBar::add(RefPtr<ui::Menu> menu)
{
    assert(menu);
    ui::CompositeMenu::add(menu);

    menu_->add(menu->name().c_str(), 0, exec_menu_item, menu.get());
}


void FlMenuBar::exec_menu_item(Fl_Widget* w, void* p)
{
    std::clog << __func__  << std::endl;
}

