//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "controller.h"
#include "dialog.h"
#if 0
 #include <iostream>
 #include <typeinfo>
 using namespace std;
#endif

using namespace ui;


Dialog::Dialog(Controller& controller)
    : controller_(controller)
{
}


Dialog::~Dialog()
{
}


void Dialog::add_action(
    const std::string&  name,
    Callback            callback )
{
    actions_[name] = callback;
}


void Dialog::exec_action(const std::string& action)
{
    auto i = actions_.find(action);
    if (i == actions_.end())
    {
        throw std::logic_error(action + ": action not found");
    }

    i->second();
}


void Dialog::close()
{
    hide();
    controller_.set_current_dialog(nullptr);

    close_impl();
}


void Dialog::update(const ui::State& s)
{
    for (auto v = begin(views_); v != end(views_); ++v)
    {
        (*v)->update(s);
    }
}


void Dialog::update_breakpoint(BreakPoint& bp)
{
    for (auto v = begin(views_); v != end(views_); ++v)
    {
        (*v)->update_breakpoint(bp);
    }
}

