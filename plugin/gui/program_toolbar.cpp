//
// $Id: program_toolbar.cpp 720 2010-10-28 06:37:54Z root $
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
#include "zdk/check_ptr.h"
#include "gtkmm/connect.h"
#include "gtkmm/flags.h"
#include "gtkmm/pixmap.h"
#include "gtkmm/stock.h"
#include "program_toolbar.h"
#include "states.h"
#include "icons/tool/back2.xpm"
#include "icons/tool/forward2.xpm"
//#include "icons/tool/step_in.xpm"
//#include "icons/tool/step_over.xpm"
//#include "icons/tool/return.xpm"
//#include "icons/tool/run2.xpm"
#include "icons/tool/stop2.xpm"
#include "icons/tool/step.xpm"
#include "icons/tool/stepi.xpm"
#include "icons/tool/run.xpm"
#include "icons/tool/next.xpm"
#include "icons/tool/ret.xpm"

using namespace std;
using namespace Gtk;

#if !defined (GTKMM_2)
 using namespace Gtk::Toolbar_Helpers;
#endif

#define STOCK_ICON(x) "gtk-" x


////////////////////////////////////////////////////////////////
ProgramToolBar::ProgramToolBar() : btnBack_(0), btnForward_(0)
{
    btnBack_ = add_button(back2_xpm,
                        "Navigate back",
                        &ProgramToolBar::on_back, 0,
                        "Back",
                        "gtk-go-back",
                        false);

    btnForward_ = add_button(
                        forward2_xpm,
                        "Navigate forward",
                        &ProgramToolBar::on_forward, 0,
                        "Forward",
                        "gtk-go-forward",
                        false);
    add_separator();

    add_button(run_xpm,
        "Evaluate expression",
        &ProgramToolBar::on_eval,
        uisAttachedLive | uisThreadStop,
        "Evaluate", "gtk-find");

    btnBack_->set_sensitive(false);
    btnForward_->set_sensitive(false);

    add_button(stop_xpm,
        "Break execution",
        &ProgramToolBar::on_stop,
        uisAttachedLive | uisThreadRun,
        "Break",
        //"gtk-media-pause");
        "gtk-stop");

    add_button(//step_over_xpm,
        stepi_xpm,
        "Execute one machine instruction",
        &ProgramToolBar::on_instr,
        uisAttachedLive | uisThreadStop,
        "Instruction",
        STOCK_ICON("go-down"));

    add_button(//step_in_xpm,
        step_xpm,
        "Step into function",
        &ProgramToolBar::on_step,
        uisAttachedLive | uisThreadStop,
        "Step",
        STOCK_ICON("goto-last"));

    add_button(//step_over_xpm,
        next_xpm,
        "Step over to next line",
        &ProgramToolBar::on_next,
        uisAttachedLive | uisThreadStop,
        "Next",
        STOCK_ICON("redo"));
        //STOCK_ICON("go-forward"));

    add_button(return_xpm,
        "Run until function returns",
        &ProgramToolBar::on_return,
        uisAttachedLive | uisThreadStop,
        "Return",
        STOCK_ICON("undo"));

    add_button(run_xpm,
        "Resume execution",
        &ProgramToolBar::on_run,
        uisAttachedLive | uisThreadStop,
        "Continue",
        STOCK_ICON("media-play")
        //STOCK_ICON("execute")
        );
}


////////////////////////////////////////////////////////////////
void ProgramToolBar::on_can_backup(size_t steps)
{
    assert(btnBack_);
    btnBack_->set_sensitive(steps != 0);
}


////////////////////////////////////////////////////////////////
void ProgramToolBar::on_can_forward(size_t steps)
{
    assert(btnForward_);
    btnForward_->set_sensitive(steps != 0);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
