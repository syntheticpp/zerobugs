//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: program_toolbar2.cpp 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#ifdef GTKMM_2
#include <gtkmm/box.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/toolitem.h>
#include <pangomm/fontdescription.h>
#include "program_toolbar2.h"
#include "text_entry.h"
#include <generic/temporary.h>
#include "zdk/zero.h"
#include "states.h"
#include <iostream>

using namespace std;


ProgramToolBar2::ProgramToolBar2(Debugger& debugger)
    : fbtn_(manage(new Gtk::FontButton))
    , entry_(manage(new TextEntry(debugger.properties(), "tool.font")))
    , inSignal_(false)
{
    set_show_arrow(true);

    fbtn_->set_title("Source Code Font");
    fbtn_->signal_font_set().connect(bind(&ProgramToolBar2::on_font_set, this));

    Gtk::Box* box = manage(new Gtk::HBox);
    box->set_border_width(3);
    box->set_spacing(3);

    box->add(*fbtn_);

    Gtk::ToolItem* item = manage(new Gtk::ToolItem);
    item->add(*box);

    item->show_all();
    box->set_data(STATE_MASK, reinterpret_cast<void*>(uisAttachedThreadStop));

    add_separator();
    append(*item);
}



void ProgramToolBar2::set_font_name(const std::string& fname)
{
    Temporary<bool>__inScope__(inSignal_, true);
    if (fbtn_)
    {
        Pango::FontDescription fontDesc(fname);

        if (fontDesc.get_size() == 0)
        {
            fbtn_->set_font_name(fname + " 10");
        }
        else
        {
            fbtn_->set_font_name(fname);
        }

        fontName_ = fbtn_->get_font_name();
    }

    if (entry_)
    {
        entry_->set_text(fontName_);
    }
}


/**
 * current selection in drop down list changed
 */
void ProgramToolBar2::on_combo_selection_change()
{
    if (!inSignal_)
    {
        Temporary<bool>__inScope__(inSignal_, true);
        string fontName = entry_->get_text(false);

        CHKPTR(fbtn_)->set_font_name(fontName);
        fontName_ = fbtn_->get_font_name();
        font_set.emit();
    }
}


/**
 * font changed as result of  pressing the font button
 */
void ProgramToolBar2::on_font_set()
{
    Temporary<bool>__inScope__(inSignal_, true);

    if (fbtn_)
    {
        fontName_ = fbtn_->get_font_name();
        CHKPTR(entry_)->set_text(fontName_);

        font_set.emit();
    }
}


void ToolBar::add_separator()
{
    Gtk::ToolItem* item = manage(new Gtk::SeparatorToolItem);
    this->add(*item);
    item->show();
}


#else

#include "program_toolbar2.h"
//
// Gtk 1.2  stubs
//
ProgramToolBar2::ProgramToolBar2(Debugger&) : fbtn_(NULL)
{
    set_space_style(GTK_TOOLBAR_SPACE_LINE);
    set_button_relief(GTK_RELIEF_NONE);
    set_style(GTK_TOOLBAR_TEXT);
}


void ProgramToolBar2::set_font_name(const std::string& name)
{
}


void ToolBar::add_separator()
{
}
#endif
