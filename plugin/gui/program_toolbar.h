#ifndef PROGRAM_TOOLBAR_H__790628B2_CE8D_41DE_9E6A_A68960171988
#define PROGRAM_TOOLBAR_H__790628B2_CE8D_41DE_9E6A_A68960171988
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
#include "toolbar.h"
#include "gtkmm/signal.h"

class ProgramToolBar : public ToolBar
{
public:
    ProgramToolBar();

    SigC::Signal0<void> tool_back;
    SigC::Signal0<void> tool_forward;

    SigC::Signal0<void> tool_break;
    SigC::Signal0<void> tool_eval;
    SigC::Signal0<void> tool_restart;
    SigC::Signal0<void> tool_return;
    SigC::Signal0<void> tool_run;
    SigC::Signal0<void> tool_step;
    SigC::Signal0<void> tool_next;
    SigC::Signal0<void> tool_instruction;

    void on_can_backup(size_t steps);
    void on_can_forward(size_t steps);

private:
    void on_back()   { tool_back(); }
    void on_forward(){ tool_forward(); }

    void on_eval()   { tool_eval(); }
    void on_return() { tool_return(); }
    void on_run()    { tool_run(); }
    void on_step()   { tool_step(); }
    void on_stop()   { tool_break(); }
    void on_next()   { tool_next(); }
    void on_instr()  { tool_instruction(); }

private:
    Gtk::Widget*    btnBack_;
    Gtk::Widget*    btnForward_;
};

#endif // PROGRAM_TOOLBAR_H__790628B2_CE8D_41DE_9E6A_A68960171988
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
