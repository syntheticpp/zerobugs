//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fl_tabbed.h"
#include <iostream>


int Fl_Tabbed::handle(int event)
{
    std::clog << __func__ << ": " << event << std::endl;
    switch (event)
    {
    }

    return Fl_Tabs::handle(event);
}

