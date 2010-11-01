//
// $Id: set_watchpoint_dlg.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <iomanip>
#include <iostream>
#include <sstream>
#include "gtkmm/box.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/connect.h"
#include "gtkmm/entry.h"
#include "gtkmm/flags.h"
#include "gtkmm/frame.h"
#include "gtkmm/label.h"
#include "gtkmm/menu.h"
#include "gtkmm/optionmenu.h"
#include "gtkmm/radiobutton.h"
#include "gtkmm/resize.h"
#include "text_entry.h"
#include "zdk/check_ptr.h"
#include "zdk/data_type.h"
#include "set_watchpoint_dlg.h"

using namespace std;
using namespace SigC;


SetWatchPointDialog::SetWatchPointDialog
(
    Properties* properties,
    RefPtr<DebugSymbol> sym,
    addr_t addr
)
  : DialogBox(btn_ok_cancel, "Memory WatchPoint")
  , RadioGroupHelper(*this, properties)
  , addrEntry_(NULL)
  , condEntry_(NULL)
  , type_(WATCH_WRITE)
  , rel_(EQ)
  , sym_(sym)
{
    Gtk_set_size(this, 400, -1);

    Gtk::Frame* frame = manage(new Gtk::Frame);
    get_vbox()->pack_start(*frame);

    Gtk::Box* box = manage(new Gtk::VBox);
    frame->add(*box);
    box->set_spacing(2);

    ostringstream buf;
    buf << "Break when memory ";
    if (sym.is_null())
    {
        Gtk::HBox* hbox = manage(new Gtk::HBox);
        box->pack_start(*hbox);
        Gtk::Label* label = manage(new Gtk::Label("Enter address to watch:", .0));
        hbox->pack_start(*label, false, false);
        addrEntry_ = manage(new TextEntry(*this, properties, "watch_addr"));

        box->pack_start(*addrEntry_);
        if (addr)
        {
            ostringstream os;
            os << hex << showbase << addr;
            addrEntry_->set_text(os.str());
        }
    }
    else
    {
        buf << " at " << hex << sym->addr() << ' ';
    }
    buf << "is ";
    string label = buf.str() + "_written";

    Gtk::Widget& b1 = create_button(label, "watch_write", &grp_);
    box->pack_start(b1);

    label = buf.str() + "_read or written";
    Gtk::Widget& b2 = create_button(label, "watch_read_write", &grp_);

    box->pack_start(b2, false, false);

    if (!sym.is_null())
    {
        if (CHKPTR(sym->type())->is_fundamental())
        {
            add_variable_condition(properties, *box, grp_);
        }
    }
    // if checked, watchpoint is global, otherwise it
    // applies to the current thread only
    Gtk::Widget& b3 = create_button("Apply to _all threads", "watch_all");
    box->pack_end(b3, false, true, 10);

    frame->set_border_width(10);
    box->set_border_width(10);
    Gtk_set_resizable(this, true);
    get_button_box()->set_layout(Gtk_FLAG(BUTTONBOX_END));

    show_all();
}


SetWatchPointDialog::~SetWatchPointDialog()
{
}


void SetWatchPointDialog::add_variable_condition(
    Properties* properties,
    Gtk::Box& box,
    Gtk::RadioButtonGroup& grp)
{
    Gtk::HBox* hbox = manage(new Gtk::HBox);
    box.pack_start(*hbox, false, true);

    hbox->set_spacing(2);

    Gtk::Widget& b =
        create_button("_Value becomes:", "watch_value", &grp);
    hbox->pack_start(b, false, false);

    // todo: use the expression evaluator
    static struct { const char* label; RelType rel; } r[] = {
        { "==", EQ },
        { "!=", NEQ },
        { "<",  LT },
        { "<=", LTE },
        { ">",  GT },
        { ">=", GTE },
    };

    Gtk::Menu* menu = manage(new Gtk::Menu);
    for (unsigned i = 0; i != 6; ++i)
    {
        Gtk::MenuItem* item = manage(new Gtk::MenuItem(r[i].label));
        menu->append(*item);
        Gtk_CONNECT_1(item, activate, this, &SetWatchPointDialog::set_rel, r[i].rel);

        item->show();
    }

    Gtk::OptionMenu* opt = manage(new Gtk::OptionMenu);
    opt->set_menu(*menu);

    hbox->pack_start(*opt, false, false);

    // add a text entry control for specifying the value
    condEntry_ = manage(new TextEntry(*this, properties, "watch_cond"));
    Gtk_set_size(condEntry_, 10, -1);
    hbox->add(*condEntry_);

    if (properties->get_word("watch_value", 0))
    {
        condEntry_->grab_focus();
    }
    Gtk_CONNECT_0(condEntry_, focus_in_event, this, &SetWatchPointDialog::on_focus_in);
}


void SetWatchPointDialog::on_toggle_impl(const char* property, bool flag)
{
    assert(property);

    if (strcmp(property, "watch_write") == 0)
    {
        type_ = WATCH_WRITE;
    }
    else if (strcmp(property, "watch_read_write") == 0)
    {
        type_ = WATCH_READ_WRITE;
    }
    else if (strcmp(property, "watch_value") == 0)
    {
        type_ = WATCH_VALUE;

        if (flag && condEntry_ && condEntry_->can_focus())
        {
            condEntry_->grab_focus();
        }
    }
}


DialogBox::ButtonID SetWatchPointDialog::run(const Gtk::Widget* w)
{
    ButtonID btnID = DialogBox::run(w);

    if (btnID == btn_ok)
    {

        if (type_ == WATCH_VALUE)
        {
            assert(condEntry_);

            const string value = condEntry_->get_text();
            set_value_watch(rel_, value);
        }
        else
        {
            addr_t addr = 0;
            if (sym_.is_null())
            {
                istringstream is(CHKPTR(addrEntry_)->get_text());
                is.unsetf(ios::basefield);

                is >> addr;
            }
            else
            {
                addr = sym_->addr();
            }
            set_memory_watch(type_, addr);
        }
    }
    return btnID;
}


void SetWatchPointDialog::on_button(ButtonID btnID)
{
    bool valid = true;

    if (type_ == WATCH_VALUE && (btnID == btn_ok))
    {
        assert(!sym_.is_null());

        string value = CHKPTR(condEntry_)->get_text();

        size_t len = CHKPTR(sym_->type())->parse(value.c_str());
        if (len != value.length())
        {
            CHKPTR(condEntry_)->select_region(len, -1);
            valid = false;
        }
    }
    if (valid)
    {
        DialogBox::on_button(btnID);
    }
}


int SetWatchPointDialog::on_focus_in(GdkEventFocus*)
{
    if (Gtk::ToggleButton* btn = get_button("watch_value"))
    {
        if (!btn->get_active())
        {
            btn->set_active(true);
        }
    }
    return 0;
}
// Copyright (c) 2004, 2005 Cristian L. Vlasceanu

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
