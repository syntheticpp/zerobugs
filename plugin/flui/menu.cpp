//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "menu.h"


RefPtr<ui::Command> ui::MenuElem::emit_command() const
{
    return nullptr;
}


void ui::CompositeMenu::add(RefPtr<MenuElem> menu) 
{
    children_.push_back(menu);
}

/*
void ui::CompositeMenu::add(

    const std::string&  name,
    int                 shortcut,
    EnableCondition     enable,
    RefPtr<Command>     command)
{
    RefPtr<ui::MenuElem> elem(new MenuItem(name, shortcut, enable, command));
    add(elem);
}
*/


void ui::CompositeMenu::update(const State& state) 
{
    for (auto i = children_.begin(); i != children_.end(); ++i)
    {
        (*i)->update(state);
    }
}


ui::MenuItem::MenuItem(

    const std::string&  name,
    int                 shortcut,
    EnableCondition     enableCond,
    RefPtr<ui::Command> command )

    : MenuElem(name, shortcut)
    , command_(command)
    , enableCond_(enableCond)
{
}


void ui::MenuItem::update(const State& state)
{
    switch (enableCond_)
    {
    case Enable_Always:
        break;

    case Enable_IfStopped:
        enable(state.is_target_stopped());
        break;

    case Enable_IfRunning:
        enable(state.is_target_running());
        break;
    }
}

