// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: main_window_gtk1.cpp 714 2010-10-17 10:03:52Z root $
//
#include <iostream>
#include "gtkmm/accelmap.h"
#include "main_window.h"

using namespace std;


void MainWindow::load_key_bindings(Properties& props)
{
    MenuAccelMap::const_iterator i = TheAccelMap::instance().begin();
    for (; i != TheAccelMap::instance().end(); ++i)
    {
        AccelKey ak(props.get_word(i->first.c_str(), 0));
        if (ak.key() == 0)
        {
            continue;
        }
    #ifdef DEBUG
        clog << __func__ << ": " << i->first << "=" << ak.abrev() << endl;
    #endif
        Gtk::AccelMap::MenuElement(*i->second).set_accel(ak);
    }

}


void MainWindow::save_key_bindings()
{
    Properties* props = debugger().properties();

    MenuAccelMap::const_iterator i = TheAccelMap::instance().begin();
    for (; i != TheAccelMap::instance().end(); ++i)
    {
        guint accelkey = i->second->accel_key;
        props->set_word(i->first.c_str(), accelkey);
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
