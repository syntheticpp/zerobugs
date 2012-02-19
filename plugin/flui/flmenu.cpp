//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "controller.h"
#include "flmenu.h"
// #include <iostream>
using namespace std;


FlMenuBar::FlMenuBar(

    ui::Controller& c,
    int             w,
    int             h )

  : ui::CompositeMenu()
  , controller_(c)
  , menu_(new Fl_Menu_Bar(0, 0, w, h))
{
}


FlMenuBar::~FlMenuBar() throw()
{
}


void FlMenuBar::add(

    const string&       name,
    int                 shortcut,
    EnableCondition     enable,
    RefPtr<ui::Command> command)
{
    int i = menu_->add(name.c_str(), shortcut, exec_command, this);
    RefPtr<ui::MenuElem> item(
        new FlMenuItem(name, shortcut, enable, command, menu_, i));
    ui::CompositeMenu::add(item);
}


void FlMenuBar::exec_command(Fl_Widget* /* w */, void* p)
{
    FlMenuBar* menubar = reinterpret_cast<FlMenuBar*>(p);

    char path[100] = { 0 };
    assert(menubar->menu_);

    menubar->menu_->item_pathname(path, sizeof(path) - 1);
    menubar->exec_command(path);
}


void FlMenuBar::exec_command(const char* path)
{
    // find the menu item that issued the command

    for (auto mi = begin(children_); mi != end(children_); ++mi)
    {
        if (strcmp((*mi)->name().c_str(), path) == 0)
        {
            controller_.call_async_on_main_thread((*mi)->emit_command());
            break;
        }
    }
}

FlMenuItem::FlMenuItem(

    const string&       name,
    int                 shortcut,
    EnableCondition     cond,
    RefPtr<ui::Command> cmd,
    Fl_Menu_Bar*        menu,
    int                 index )

    : ui::MenuItem(name, shortcut, cond, cmd)
    , menu_(menu)
    , index_(index)
{
}


void FlMenuItem::enable(bool activate)
{
    const int flags = menu_->mode(index_);

    if (activate)
    {
        menu_->mode(index_, flags & ~FL_MENU_INACTIVE);
    }
    else
    {
        menu_->mode(index_, flags | FL_MENU_INACTIVE);
    }
}


