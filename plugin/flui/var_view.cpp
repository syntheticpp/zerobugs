//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "controller.h"
#include "flvar_view.h"
#include "var_view.h"
#include <iostream>


ui::VarView::VarView(ui::Controller& c) : View(c)
{
}


ui::VarView::~VarView() throw()
{
}


bool ui::VarView::notify(DebugSymbol* s)
{
    if (s)
    {
        std::clog << s->name() << std::endl;
    }
    return true;
}
