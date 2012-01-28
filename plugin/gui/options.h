#ifndef OPTIONS_H__ED4F9FCC_0F56_4924_85B6_6E93AC684298
#define OPTIONS_H__ED4F9FCC_0F56_4924_85B6_6E93AC684298
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
#include <map>
#include <vector>
#include <string>
#include "zdk/disasm.h"
#include "gtkmm/box.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/notebook.h"
#include "gtkmm/radiobutton.h"
#include "filter_param.h"

class DataFilter;
class Debugger;
class MainWindow;
class ProgramView;

namespace Gtk
{
    class Box;
    class CheckButton;
    class FontSelection;
    class SpinButton;
}
class TextEntry;
class Variant;


/**
 * User Interface element for configuring miscellaneous options.
 */
class ZDK_LOCAL Options
    : public Notebook_Adapt
    , EnumCallback3<const char*, const char*, const Variant*, bool>
{
    typedef std::vector<Gtk::CheckButton*> ButtonList;
    typedef std::map<std::string, TextEntry*> AccelEntryMap;

    // parameters for the DataFilter plugin (if any is present)
    typedef boost::shared_ptr<DataFilterParam> FilterParamPtr;
    typedef std::vector<FilterParamPtr> FilterParamList;

public:
    explicit Options(Debugger&);

    void apply_signal_handling();
    void apply_fonts(MainWindow*, const std::string&);
    void apply_options(MainWindow*);
    void apply_layout(MainWindow*);
    void apply_accels(MainWindow*);
    void apply_asm_syntax();
    void apply_filter_params(DataFilter* = 0);

    Gtk::FontSelection* font_selection() { return fontSel_; }

private:
    /**
     * adds a dialog page where the user can modify the
     * way signals are handled
     */
    void add_signals_page();

    /**
     * add a page for customizing accelerators
     */
    void add_accel_page();

    void add_accel_entry(Gtk::Box&, const char* name);

    /**
     * add page with options that customize the disassembler
     */
    void add_disasm_page();
    void add_disasm_syntax( Gtk::Box&,
                            const char* name,
                            Disassembler::Syntax,
                            Disassembler::Syntax);

    void set_asm_syntax(Gtk::RadioButton*, Disassembler::Syntax);

    void add_layout_page();

    void add_layout_controls(Gtk::Box&);
    void add_layout_entry(Gtk::Box&, const char* label, std::string);
    void set_layout_name(Gtk::RadioButton*, std::string);
    void add_tab_size_control(Gtk::Box&);

    /**
     * language- and runtime-specific settings
     */
    void add_lang_page();

    /**
     * add misc page, this is a catch-all, customize things that
     * do not fit anywhere else
     */
    void add_misc_page();

    /**
     * add a page for customizing the fonts used to display
     * source code
     */
    void add_fonts_page();

    void set_current_page_title(const char*);

    Gtk::Box& add_vbox(const char* tabLabel);

    void add_data_filter_params();

    /**
     * works in conjunction with DataFilter::enum_params()
     */
    bool notify(const char* paramName,
                const char* descriptiveLabel,
                const Variant* defaultValue);

    void on_trace_fork(Gtk::ToggleButton*);

private:
    Debugger& debugger_;
    ButtonList pass_; // for signal policies
    ButtonList stop_; // ditto

    Gtk::CheckButton* exceptBtn_;
    Gtk::CheckButton* mainBtn_;
    Gtk::CheckButton* traceForkBtn_;
    Gtk::CheckButton* spawnForkBtn_;
    Gtk::CheckButton* saveTabsBtn_;
    Gtk::CheckButton* confirmQuitBtn_;
    Gtk::CheckButton* confirmNextBtn_;
    Gtk::FontSelection* fontSel_;
    std::string layoutCurrent_,
                layoutName_;
    Gtk::RadioButtonGroup layoutGroup_;
    AccelEntryMap accelEntryMap_;

    Gtk::RadioButtonGroup asmGroup_;
    Disassembler::Syntax asmSyntax_;
    Gtk::Box* filterBox_;
    FilterParamList filterParams_;
    Gtk::SpinButton* tab_;
};



ZDK_LOCAL Gtk::CheckButton* new_check_button(const char*);
//ZDK_LOCAL Gtk::CheckButton* new_check_button(Debugger&, uint64_t, const char*);


#endif // OPTIONS_H__ED4F9FCC_0F56_4924_85B6_6E93AC684298
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
