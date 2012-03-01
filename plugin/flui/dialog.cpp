//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "controller.h"
#include "dialog.h"
using namespace ui;


Dialog::Dialog( Controller& controller )
    : controller_(controller)
{
}


Dialog::~Dialog()
{
    //TODO: support stacked dialogs?
    controller_.set_current_dialog(nullptr);
}


void Dialog::add_action(

    const std::string&  name,
    Callback            callback )

{
    actions_[name] = callback;
}

