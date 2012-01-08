//
// $Id: right_menu.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "gui.h" // dbgout
#include "code_view.h"
#include "right_menu.h"


MenuRunToCursor::MenuRunToCursor()
    : Base(&CodeView::run_to_cursor, "Run To Cursor", &Gtk::Stock::JUMP_TO)
{
}


bool
MenuRunToCursor::add_to(
    CodeView*           view,
    Gtk::Menu&          menu,
    MenuClickContext&   ctxt)
{
    if (ctxt.position() && 
        view            &&
        view->right_click().nearest_addr() &&
        Base::add_to(view, menu, ctxt))
    {
        menu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
        return true;
    }
    return false;
}



MenuInsertBreakpoint::MenuInsertBreakpoint()
    : Base(&CodeView::insert_breakpoint, "Insert Breakpoint", &Gtk::Stock::DIALOG_ERROR)
{
}


bool
MenuInsertBreakpoint::add_to(CodeView* view, Gtk::Menu& menu, MenuClickContext& ctxt)
{
    assert(view);

    if (ctxt.position() == 0
        || view->context_menu_add_breakpoint_items(menu)
       )
    {
        return false;
    }
    return Base::add_to(view, menu, ctxt);
}



MenuSetProgramCounter::MenuSetProgramCounter()
    : Base(&CodeView::set_program_counter, "Set Program Counter Here", NULL)
{
}


bool
MenuSetProgramCounter::add_to(
    CodeView*           view,
    Gtk::Menu&          menu,
    MenuClickContext&   ctxt)
{
    if (ctxt.position() && 
        view            &&
        view->right_click().nearest_addr() &&
        Base::add_to(view, menu, ctxt))
    {
        menu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
        return true;
    }
    return false;
}


MenuShowFunctionStart::MenuShowFunctionStart()
    : Base(&CodeView::show_function_start, "Show Function Start", &Gtk::Stock::GOTO_TOP)
{
}


MenuShowNextStatement::MenuShowNextStatement()
    : Base(&CodeView::show_next_statement, "Show Next Statement", &Gtk::Stock::GOTO_LAST)
{
}


MenuOpenFolder::MenuOpenFolder()
    : Base(&CodeView::open_folder, "Open Containing Folder", NULL)
{
}

bool
MenuOpenFolder::add_to(
    CodeView*           view,
    Gtk::Menu&          menu,
    MenuClickContext&   ctxt)
{
    if (view && view->file() && Base::add_to(view, menu, ctxt))
    {
        menu.items().push_back(Gtk::Menu_Helpers::SeparatorElem());
        return true;
    }
    return false;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
