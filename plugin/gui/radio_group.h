#ifndef RADIO_GROUP_H__A7D936B0_B30A_44B5_9ECF_25EBAC410A54
#define RADIO_GROUP_H__A7D936B0_B30A_44B5_9ECF_25EBAC410A54
//
// $Id: radio_group.h 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <string>
#include <map>
#include "gtkmm/base.h"
#include "gtkmm/widget.h"
#include "gtkmm/radiobutton.h"
#ifdef __GNUC__
  #define interface
#endif


class Properties;

namespace Gtk
{
    class ToggleButton;
    class Widget;
    class Window;
}


class RadioGroupHelper : public Gtk::Base
{
public:
    RadioGroupHelper(Gtk::Window&, Properties*);

    virtual ~RadioGroupHelper();

protected:
    Gtk::Widget& create_button(
        const std::string& name,
        const char* property,
        Gtk::RadioButtonGroup* = 0);

    Properties* properties() { return prop_; }

    void on_toggle(const char*);

    virtual void on_toggle_impl(const char*, bool) {};

    Gtk::ToggleButton* get_button(const char*) const;

private:
    typedef std::map<std::string, Gtk::ToggleButton*> ButtonMap;

    Gtk::Window& wnd_;
    Properties* prop_;

    ButtonMap map_;
};

#endif // RADIO_GROUP_H__A7D936B0_B30A_44B5_9ECF_25EBAC410A54
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
