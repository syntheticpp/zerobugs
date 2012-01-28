#ifndef DIALOG_BOX_H__D0148851_FDC9_4E16_9D0A_03A5FC7C3ED1
#define DIALOG_BOX_H__D0148851_FDC9_4E16_9D0A_03A5FC7C3ED1
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
#include "gtkmm/dialog.h"
#include "gtkmm/events.h"
#include "gtkmm/icon_mapper.h"
#include "gtkmm/signal.h"
#include "gtkmm/stock.h"
#include <boost/utility.hpp>

namespace Gtk
{
    class Button;
    class ButtonBox;
}


CLASS DialogBox : public OnMapEventImpl<Gtk::Dialog>, boost::noncopyable
{
public:
    enum ButtonID
    {
        btn_none                = 0x0000,
        btn_ok                  = 0x0001,
        btn_yes                 = 0x0002,
        btn_all                 = 0x0004,
        btn_no                  = 0x0008,
        btn_cancel              = 0x0010,
        btn_close               = 0x0020,

        btn_ok_close            = btn_ok | btn_close,
        btn_ok_cancel           = btn_ok | btn_cancel,
        btn_ok_cancel_all       = btn_ok | btn_cancel | btn_all,
        btn_yes_no              = btn_yes| btn_no,
        btn_yes_no_cancel       = btn_yes_no | btn_cancel,
        btn_yes_all_no_cancel   = btn_yes_no_cancel | btn_all
    };

    DialogBox(ButtonID, const char* title);

    virtual ~DialogBox();

    ButtonID run(const Gtk::Widget* = 0, bool modal = true);

    ButtonID run(int x, int y);

    void center(gint x, gint y, gint w=-1, gint h=-1);

    void center(const Gtk::Widget&);

    Gtk::Button* get_ok_button() { return okBtn_; }

    virtual void on_button(ButtonID);

    Gtk::Button* add_button(const char* text)
    { return add_button(text, get_button_box()); }

    static Gtk::Dialog* active_dialog() { return active_; }

    SigC::Signal0<void> dialog_closed_event;

    virtual void close_dialog();

    void add_button_accelerator(Gtk::Button&, int = 0);

protected:
    Gtk::Button* add_button(const char* text, Gtk::ButtonBox*);
    Gtk::Button* add_button(const Gtk::StockID&, Gtk::ButtonBox*);

    Gtk::ButtonBox* get_button_box() { return bbox_; }

    ButtonID result() const { return result_; }

    event_result_t on_delete_event(GdkEventAny*);

#ifdef GTKMM_2
    void set_uposition(int x, int y) { move(x, y); }

#endif

private:
    Gtk::ButtonBox*     bbox_;
    Gtk::Button*        okBtn_; // ok button, if specified
    ButtonID            result_;
    static Gtk::Dialog* active_;
    Gtk::Dialog*        oldActive_;
};

#endif // DIALOG_BOX_H__D0148851_FDC9_4E16_9D0A_03A5FC7C3ED1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
