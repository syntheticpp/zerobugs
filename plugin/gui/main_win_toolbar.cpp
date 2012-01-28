//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include "dharma/sarray.h"
#include "gtkmm/connect.h"
#include "gtkmm/menu.h"
#include "zdk/command.h"
#include "entry_dialog.h"
#include "gui.h"
#include "main_window.h"
#include "program_toolbar2.h"
#include "slot_macros.h"
#include <boost/tokenizer.hpp>

using namespace Gtk;
using namespace std;



ToolBar* MainWindow::get_toolbar(Properties& prop)
{
    // use the main toolbar by default
    ToolBar* toolbar = toolbar_;

    const char* tool = prop.get_string("tool");
    if (tool && *tool)
    {
        ToolMap::iterator i = toolMap_.find(tool);
        if (i == toolMap_.end())
        {
            toolbar = manage(new ToolBar);
            add_toolbar(tool, *toolbar);
            toolMap_.insert(i, make_pair(tool, toolbar));
        }
        else
        {
            toolbar = i->second;
            if (prop.get_word("separator", 0))
            {
                toolbar->add_separator();
            }
        }
    }
    return toolbar;
}



void
MainWindow::add_toolbar_button(DebuggerCommand* cmd,
                               const string& tip,
                               const char* stock,
                               SArray& pixmap,
                               Properties& prop)
{
    ToolBar* toolbar = get_toolbar(prop);

    if (pixmap.empty())
    {
        pixmap.push_back("");
    }
    SArray noArg;

    Widget* btn =
        CHKPTR(toolbar)->add_button(
            pixmap.cstrings(),
            tip.c_str(),
            Gtk_BIND(Gtk_SLOT(this, &MainWindow::run_macro), cmd, noArg),
            uisAttachedThreadStop, // FIXME: let the caller specify it
            cmd->name(),
            stock);

    btn->show_all();
    commandMap_[cmd] = btn;
}



void
MainWindow::toolbar_entry_dialog(DebuggerCommand* command,
                                 Properties* cmdProperties)
{
    const string msg = cmdProperties->get_string("message", "");
    EntryDialog dialog(msg, debugger().properties(), command->name());

    SArray args;

    string userString = dialog.run();
    if (!userString.empty())
    {
        args.push_back(userString);
        run_macro(command, args);
    }
}


void
MainWindow::add_toolbar_dialog(DebuggerCommand* cmd,
                               const string& tip,
                               const char* stock,
                               SArray& pixmap,
                               Properties& prop)
{
    ToolBar* toolbar = get_toolbar(prop);

    if (pixmap.empty())
    {
        pixmap.push_back("");
    }

    Widget* btn =
        CHKPTR(toolbar)->add_button(
            pixmap.cstrings(),
            tip.c_str(),
            Gtk_BIND(Gtk_SLOT(this, &MainWindow::toolbar_entry_dialog),
                     cmd, &prop
                    ),
            uisAttachedThreadStop, // FIXME: let the caller specify it
            cmd->name(),
            stock);

    btn->show_all();
    commandMap_[cmd] = btn;
}


void MainWindow::on_can_navigate_back(size_t numSteps)
{
    if (is_at_debug_event())
    {
        toolbar_->on_can_backup(numSteps);
    }
    else
    {
        toolbar_->on_can_backup(0);
    }
}


void MainWindow::on_can_navigate_forward(size_t numSteps)
{
    if (is_at_debug_event())
    {
        toolbar_->on_can_forward(numSteps);
    }
    else
    {
        toolbar_->on_can_forward(0);
    }
}


bool
MainWindow::add_toolbar(const char* name, ToolBar& toolbar, bool defaultVisibility)
{
    assert(name);

    if (toolMap_.insert(make_pair(name, &toolbar)).second)
    {
        CHKPTR(toolbox_)->pack_start(toolbar, false, false);

        string toolName(name);
        MenuList& items = CHKPTR(toolMenu_)->items();

        items.push_back(Gtk::Menu_Helpers::CheckMenuElem(
            toolName,
            Gtk_BIND(Gtk_SLOT(this, &MainWindow::on_menu_toggle_toolbar),
                     &toolbar)));
        toolName = "toolbar." + toolName;

        if (debugger().properties()->get_word(toolName.c_str(), defaultVisibility))
        {
            Gtk_MENU_ITEM(items.back()).activate();
            toolbar.show_all();
        }
        return true;
    }
    return false;
}



void MainWindow::add_command(DebuggerCommand* cmd)
{
    if (!is_ui_thread())
    {
        run_on_ui_thread(command(&MainWindow::add_command, this, cmd));
    }
    else
    {
        dbgout(0) << __func__ << ": " << cmd->name() << endl;
        //
        // The command object may implement a Properties interface,
        // (a dictionary, essentially) which allows the various
        // CommandCenter implementations to extract the parameters
        // that apply.
        //
        // In this particular case, we look for params that tell:
        // 1) whether the command to be added is a toolbar button
        // (other types are not supported yet, but things such as
        // menu items may are on the drawing board and may be added soon),
        // and:
        // 2) if a toolbar button, what are its attributes (tooltip
        // text, what pixmap or stock image to show, etc).
        //
        if (Properties* prop = interface_cast<Properties*>(cmd))
        {
            SArray pixmap;

            string tip = prop->get_string("tooltip", "");
            const char* stock = prop->get_string("stock");

            if (const char* xpm = prop->get_string("pixmap"))
            {
                typedef boost::char_separator<char> Delim;
                typedef boost::tokenizer<Delim> Tokenizer;

                string tmp(xpm);
                Tokenizer tok(tmp, Delim("\n"));

                Tokenizer::iterator i = tok.begin();
                for (; i != tok.end(); ++i)
                {
                    pixmap.push_back(*i);
                }
            }

            if (const char* type = prop->get_string("type"))
            {
                if (strcmp(type, "tool") == 0)
                {
                    add_toolbar_button(cmd, tip, stock, pixmap, *prop);
                }
                else if (strcmp(type, "tool-dialog") == 0)
                {
                    add_toolbar_dialog(cmd, tip, stock, pixmap, *prop);
                }
                else
                {
                    throw runtime_error("pluggable command type \""
                                        + string(type)
                                        + "\" not supported");
                }
            }
        }
    }
}
