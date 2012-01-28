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
#include <cassert>
#include "gtkmm/box.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/connect.h"
#include "gtkmm/label.h"
#include "gtkmm/style.h"

#include "check_listitem.h"

using namespace std;

namespace
{
    class ListCheckButton : public Gtk::CheckButton
    {
    public:

        ListCheckButton() : Gtk::CheckButton() {}

    protected:
        /* overidden, so that items are not hilighted as
           the mouse hovers over the list */
        void enter_impl() {}
        void leave_impl() {}

        /* overidden, so that we don't draw a rectangle
           around the check button */
        void grab_focus_impl() {}

        void draw_indicator_impl(GdkRectangle* rc)
        {
            Gtk::Widget* parent = get_parent();
            if (parent && parent->get_state() == GTK_STATE_SELECTED)
            {
                set_state(GTK_STATE_SELECTED);
            }
            else if (get_state() == GTK_STATE_PRELIGHT)
            {
                set_state(GTK_STATE_NORMAL);
            }
            CheckButton::draw_indicator_impl(rc);
        }
    };
} // namespace



CheckListItem::CheckListItem(const string& text, bool checked)
    : ListItem()
    , cbtn_(new ListCheckButton)
{
    Gtk::Box* box = manage(new Gtk::HBox);
    add(*box);

    box->pack_start(*cbtn_, false, false);
    box->pack_start(*manage(new Gtk::Label(text, 0.0)));

    if (checked)
    {
        cbtn_->activate();
    }
    Gtk_CONNECT(cbtn_, toggled, toggled.slot());
}


CheckListItem::~CheckListItem()
{
}


void CheckListItem::set_data(const RefPtr<ZObject>& data)
{
    data_ = data;
}


bool CheckListItem::is_checked() const
{
    assert(cbtn_.get());
    return cbtn_->get_active();
}


void CheckListItem::check(bool checkFlag)
{
    assert(cbtn_.get());
    cbtn_->set_active(checkFlag);
}


void CheckListItem::hide_check_button()
{
    cbtn_.reset();
}


bool CheckListItem::has_check_button() const
{
    return cbtn_.get();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
