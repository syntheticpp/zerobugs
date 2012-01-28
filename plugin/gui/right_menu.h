#ifndef RIGHT_MENU_H__A3A70DFB_FC78_47A8_BA9A_1C26766271A4
#define RIGHT_MENU_H__A3A70DFB_FC78_47A8_BA9A_1C26766271A4
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
#include "menu_action_impl.h"

class CodeView;


/**
 * Adds a "Run To Cursor" menu to a contextual mennu
 */
class MenuRunToCursor : public BaseMenuActionImpl<CodeView, true>
{
    typedef BaseMenuActionImpl<CodeView, true> Base;
    bool add_to(CodeView*, Gtk::Menu&, MenuClickContext&);

public:
    MenuRunToCursor();
};


class MenuInsertBreakpoint : public BaseMenuActionImpl<CodeView, true>
{
    typedef BaseMenuActionImpl<CodeView, true> Base;

    bool add_to(CodeView*, Gtk::Menu&, MenuClickContext&);

public:
    MenuInsertBreakpoint();
};



class MenuSetProgramCounter : public BaseMenuActionImpl<CodeView, true>
{
    typedef BaseMenuActionImpl<CodeView, true> Base;

    bool add_to(CodeView*, Gtk::Menu&, MenuClickContext&);

public:
    MenuSetProgramCounter();
};



class MenuShowFunctionStart : public BaseMenuActionImpl<CodeView>
{
    typedef BaseMenuActionImpl<CodeView> Base;

public:
    MenuShowFunctionStart();
};



class MenuShowNextStatement : public BaseMenuActionImpl<CodeView>
{
    typedef BaseMenuActionImpl<CodeView> Base;

public:
    MenuShowNextStatement();
};


class MenuOpenFolder : public BaseMenuActionImpl<CodeView>
{
    typedef BaseMenuActionImpl<CodeView> Base;

public:
    MenuOpenFolder();
    bool add_to(CodeView*, Gtk::Menu&, MenuClickContext&);
};

#endif // RIGHT_MENU_H__A3A70DFB_FC78_47A8_BA9A_1C26766271A4
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
