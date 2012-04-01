//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/breakpoint.h"
#include "zdk/symbol.h"
#include "zdk/switchable.h"
#include "breakpoint_view.h"
#include "controller.h"
#include "fledit_breakpoint_dlg.h"
#include "utils.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Output.H>
#include <FL/Enumerations.H>
//#include <iostream>
#include <sstream>

using namespace std;


FlEditBreakPointDlg::FlEditBreakPointDlg(

    ui::Controller& controller )

    : FlDialog(controller, 0, 0, 500, 350, "Edit Breakpoint")
    , condition_(new Fl_Input(20, 66, 460, 22))
    , descr_(static_text(20, 12, 460, 22))

{
    group()->box(FL_EMBOSSED_FRAME);

    static_text(20, 44, 460, 22, "Condition");

    //
    // OK button applies changes
    //
    add_button(305, 310, 85, 22, "&OK", [this] {
        if (auto* s = interface_cast<Switchable*>(ubp_.action.get()))
        {
            s->set_activation_expr(condition_->value());
            this->controller().awaken_main_thread();
        }
        close();
    });

    add_button(395, 310, 85, 22, "&Cancel", [this] {
        close();
    });

    center();
}


void FlEditBreakPointDlg::popup(

    const ui::State&    state,
    ui::UserBreakPoint& ubp )

{
    ubp_ = ubp;

    if (auto* s = interface_cast<Switchable*>(ubp_.action.get()))
    {
        condition_->value(s->activation_expr());
    }
    FlDialog::popup(state);
    controller().awaken_main_thread();
}


void FlEditBreakPointDlg::update_breakpoint(BreakPoint& bp)
{
    if (bp.addr() != ubp_.bpoint->addr())
    {
        return;
    }

    if (RefPtr<Symbol> sym = bp.symbol())
    {
        ostringstream oss;

        oss << basename(sym->file()->c_str());

        if (auto line = sym->line())
        {
            oss << ":" << line;
        }
        else
        {

            oss << ": " << sym->demangled_name();
            oss << " (0x" << hex << bp.addr() << ")";
        }

        descr_->value(oss.str().c_str());
    }
}

