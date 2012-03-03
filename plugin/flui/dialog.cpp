//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
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


void Dialog::close()
{
    controller_.set_current_dialog(nullptr);
}


void Dialog::update(const ui::State& s)
{
    for (auto v = begin(views_); v != end(views_); ++v)
    {
        (*v)->update(s);
    }
}

