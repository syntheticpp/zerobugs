// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "gtkmm/accelmap.h"
#include "main_window.h"


////////////////////////////////////////////////////////////////
void MainWindow::load_key_bindings(Properties& props)
{
    keyBindings_ = props.get_string("config_path", ".zero/");
    keyBindings_ += "keybind.rc";
    Gtk::AccelMap::load(keyBindings_);
}


////////////////////////////////////////////////////////////////
void MainWindow::save_key_bindings()
{
    Gtk::AccelMap::save(keyBindings_);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
