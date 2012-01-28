//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "menu.h"


std::unique_ptr<ui::Command> ui::MenuItem::emit() const
{
    return nullptr;
}

