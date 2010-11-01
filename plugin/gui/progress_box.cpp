//
// $Id: progress_box.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <assert.h>
#include <iostream>
#include "generic/lock.h"
#include "gtkmm/box.h"
#include "gtkmm/label.h"
#include "gtkmm/progressbar.h"
#include "gtkmm/resize.h"
#include "icons/question.xpm"
#include "message_box.h"
#include "progress_box.h"


using namespace std;

class ProgressBox::Bar : public Gtk::ProgressBar
{
};


////////////////////////////////////////////////////////////////
ProgressBox::ProgressBox(const string& msg, const char* title)
    : DialogBox(btn_cancel, title)
    , label_(manage(new Gtk::Label(msg, .0)))
    , bar_(manage(new Bar))
    , cancelled_(false)
    , done_(false)
{
    Gtk::VBox* box = manage(new Gtk::VBox());
    Gtk::HBox* hbox = manage(new Gtk::HBox());
    get_vbox()->pack_start(*box, false, false);

    box->set_border_width(10);
    hbox->pack_start(*label_, false, false);
    box->pack_start(*hbox);
    box->add(*bar_);

    Gtk_set_size(this, 480, 110);
    set_default_size(480, 110);
}


////////////////////////////////////////////////////////////////
void
ProgressBox::update(Gtk::Window& w, const string& msg, double perc)
{
    assert(perc <= 1.0);
    if (perc >= 1.0)
    {
        perc = 1.0;
        done_ = true;
    }

    if (done_ || cancelled_)
    {
        hide_all();
    }
    else if (!cancelled_)
    {
        label_->set_text(msg);
#if GTKMM_2
        bar_->set_fraction(perc);
#else
        bar_->set_percentage(perc);
        if (static_cast<int>(perc * 100) % 10 == 0)
        {
            bar_->draw(NULL);
            this->draw(NULL);
        }
#endif
        if (!is_visible() || !is_mapped())
        {
            set_transient_for(w);
            set_modal(true);
            show_all();
            center(w);
        }
    }
}


////////////////////////////////////////////////////////////////
void ProgressBox::close_dialog()
{
    static const char msg[] = "Cancelling this operation will"
        " prevent the debugger from accessing important\n"
        " information in your program. Do you still want to cancel?";
    MessageBox dlg(msg, MessageBox::btn_yes_no, 0, question_xpm);

    if (dlg.run() == btn_yes)
    {
        Lock<Mutex> lock(mutex_);
        hide_all();
        cancelled_ = true;
    }
}


////////////////////////////////////////////////////////////////
event_result_t ProgressBox::delete_event_impl(GdkEventAny*)
{
    close_dialog();
    return 1;
}


////////////////////////////////////////////////////////////////
void ProgressBox::reset()
{
    Lock<Mutex> lock(mutex_);
    cancelled_ = done_ = false;
}


////////////////////////////////////////////////////////////////
bool ProgressBox::cancelled() const
{
    Lock<Mutex> lock(mutex_);
    return cancelled_;
}


////////////////////////////////////////////////////////////////
bool ProgressBox::done() const
{
    Lock<Mutex> lock(mutex_);
    return done_;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
