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
#include <iostream>
#include "check_listitem.h"


using namespace std;

bool is_checked(const Gtk::ListItem* item)
{
    assert(item);

    bool result = false;

    const CheckListItem* cli = dynamic_cast<const CheckListItem*>(item);
    if (cli)
    {
        result = cli->is_checked();
    }
    return result;
}


void check(Gtk::ListItem* item, bool flag)
{
    assert(item);

    if (CheckListItem* cli = dynamic_cast<CheckListItem*>(item))
    {
        cli->check(flag);
    }
}


bool has_check_button(const Gtk::ListItem* item)
{
    assert(item);

    bool result = false;

    const CheckListItem* cli = dynamic_cast<const CheckListItem*>(item);
    if (cli)
    {
        result = cli->has_check_button();
    }

    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
