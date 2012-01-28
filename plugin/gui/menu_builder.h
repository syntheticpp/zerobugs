#ifndef MENU_BUILDER_H__7CEF78CC_1A9E_4B0B_AE36_3F87E9694484
#define MENU_BUILDER_H__7CEF78CC_1A9E_4B0B_AE36_3F87E9694484
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

#include "menu_click.h"


template<typename T>
struct ZDK_LOCAL MenuBuilder
{
    virtual ~MenuBuilder() { }

    virtual void populate(T*, Gtk::Menu&, MenuClickContext&) = 0;
};

#endif // MENU_BUILDER_H__7CEF78CC_1A9E_4B0B_AE36_3F87E9694484
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
