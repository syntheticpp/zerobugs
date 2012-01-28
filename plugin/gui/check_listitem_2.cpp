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
// Gtkmm-2.x specific part
//
#include <cassert>
#include "gtkmm/connect.h"
#include "gtkmm/label.h"
#include "gtkmm/style.h"
#include "check_listitem.h"

using namespace std;


CheckListItem::CheckListItem(const string& text, bool checked)
    : ListItem(text)
    , checked_(checked)
{
}

CheckListItem::CheckListItem(const CheckListItem& other)
    : checked_(other.checked_)
{
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
    return checked_;
}

void CheckListItem::check(bool checked)
{
    checked_ = checked;
}

void CheckListItem::hide_check_button()
{
}

bool CheckListItem::has_check_button() const
{
    return true;
}


/*
Gtk::ListItem* CheckListItem::clone()
{
    return new CheckListItem(*this);
}
*/
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
