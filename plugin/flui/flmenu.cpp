//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "flmenu.h"
#include <iostream>
#include <FL/Fl_Window.H>


FlMenuBar::FlMenuBar(

    ui::Controller& c,
    Fl_Window* w

) : ui::CompositeMenu(c)
  , menu_(new Fl_Menu_Bar(0, 0, w->w(), 30))
{
#if DEBUG
    std::clog << __PRETTY_FUNCTION__ << std::endl;
#endif
}


FlMenuBar::~FlMenuBar() throw()
{
}


void FlMenuBar::add(RefPtr<ui::Menu> menu)
{
    assert(menu);
    ui::CompositeMenu::add(menu);

//#if DEBUG
//    std::clog << __func__ << ": " << menu->name() << std::endl;
//#endif
    // meh
    menu_->add(menu->name().c_str(), 0, exec_menu_item);
}


/*
void FlMenuBar::add(RefPtr<ui::MenuItem> item)
{
    assert(menu_);

    assert(item);
    ui::CompositeMenu::add(item);

#if DEBUG
    std::clog << __func__ << ": " << item->name() << std::endl;
#endif
    menu_->add(item->name().c_str(), 0, exec_menu_item);
}
*/

void FlMenuBar::exec_menu_item(Fl_Widget* w, void* p)
{
    std::clog << __PRETTY_FUNCTION__ << std::endl;
}

