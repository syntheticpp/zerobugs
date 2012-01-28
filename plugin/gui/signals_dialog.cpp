//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <signal.h>
#include <iomanip>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/buttonbox.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "gtkmm/resize.h"
#include "gtkmm/scrolledwindow.h"
#include "gtkmm/table.h"
#include "dharma/sigutil.h"
#include "zdk/signal_policy.h"
#include "zdk/zero.h"
#include "signals_dialog.h"

using namespace std;


const char* titles[] =
{
    "\nSignal",
    "\nPass to debuggee",
    "\nStop in debugger"
};


////////////////////////////////////////////////////////////////
SignalsDialog::SignalsDialog(Debugger& debugger)
    : DialogBox(btn_ok_cancel, "Signal Handling")
    , debugger_(debugger)
{
    Gtk::Table* table = manage(new Gtk::Table(2, 3));
    get_vbox()->add(*table);

    for (int i = 0; i != 3; ++i)
    {
        Gtk::Label* label = manage(new Gtk::Label(titles[i], .0));
        // label->set_justify(Gtk_FLAG(JUSTIFY_LEFT));
        // label->set_usize(100, -1);
        table->attach(*label, i, i + 1, 0, 1,
            Gtk_FLAG(ATTACH_NONE), Gtk_FLAG(ATTACH_NONE));
    }

    Gtk::ScrolledWindow* sw = manage(new Gtk::ScrolledWindow());
    table->attach(*sw, 0, 3, 1, 2);

    table = manage(new Gtk::Table(_NSIG - 1, 3, true));
    Gtk_set_size(table, 300, -1);

    Gtk_add_with_viewport(sw, *table);
    Gtk_set_size(sw, 360, 300);

    sw->set_policy(Gtk_FLAG(POLICY_AUTOMATIC), Gtk_FLAG(POLICY_AUTOMATIC));

    for (int i = 1; i != _NSIG; ++i)
    {
        ostringstream os;

        os << setw(2) << i << "  " << sig_name(i);

        // --- label with signal's name
        Gtk::Label* label = manage(new Gtk::Label(os.str(), .0, .5));
        label->set_justify(Gtk_FLAG(JUSTIFY_LEFT));

        table->attach(*label, 0, 1, i - 1, i,
            Gtk_FLAG(FILL), Gtk_FLAG(ATTACH_NONE));

        // --- "Pass" button
        Gtk::CheckButton* btn = manage(new Gtk::CheckButton("Pass", .0));
        pass_.push_back(btn);

        if (debugger_.signal_policy(i)->pass())
        {
            btn->set_active(true);
        }
        table->attach(*btn, 1, 2, i - 1, i,
            Gtk_FLAG(EXPAND), Gtk_FLAG(ATTACH_NONE));

        // --- "Stop" button
        btn = manage(new Gtk::CheckButton("Stop", .0));
        stop_.push_back(btn);

        if (debugger_.signal_policy(i)->stop())
        {
            btn->set_active(true);
        }
        table->attach(*btn, 2, 3, i - 1, i,
            Gtk_FLAG(EXPAND), Gtk_FLAG(ATTACH_NONE));
    }

    assert(pass_.size() == stop_.size());
    assert(pass_.size() == _NSIG - 1);

    get_button_box()->set_layout(Gtk_FLAG(BUTTONBOX_END));
}



////////////////////////////////////////////////////////////////
void SignalsDialog::run(Gtk::Widget* w)
{
    if (DialogBox::run(w) == btn_ok)
    {
        for (int i = 1; i != _NSIG; ++i)
        {
            SignalPolicy* policy = debugger_.signal_policy(i);
            assert(policy);

            try
            {
                policy->set_stop(stop_[i - 1]->get_active());
                policy->set_pass(pass_[i - 1]->get_active());

            }
            catch (...)
            {
                stop_[i - 1]->set_active(policy->stop());
                pass_[i - 1]->set_active(policy->pass());

                throw;
            }
        }
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
