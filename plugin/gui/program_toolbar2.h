#ifndef PROGRAM_TOOLBAR_2_H__4AF2B974_CCC8_49CC_949A_2F47D645D531
#define PROGRAM_TOOLBAR_2_H__4AF2B974_CCC8_49CC_949A_2F47D645D531
//
// $Id: program_toolbar2.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <string>
#include "program_toolbar.h"

class ZObject;
class Debugger;
class TextEntry;

namespace Gtk
{
    class FontButton;
}

/**
 * The main toolbar.
 *
 * GTKMM 2 tweaks: add font selection button, etc
 */
class ZDK_LOCAL ProgramToolBar2 : public ProgramToolBar
{
public:
    explicit ProgramToolBar2(Debugger&);

    void set_font_name(const std::string&);

    const std::string& font_name() const { return fontName_; }

    SigC::Signal0<void> font_set;

private:
    void on_combo_selection_change();
    void on_font_set();

    Gtk::FontButton* fbtn_;
    TextEntry* entry_;
    std::string fontName_;
    bool inSignal_;
};

#endif // PROGRAM_TOOLBAR_2_H__4AF2B974_CCC8_49CC_949A_2F47D645D531
