//
// $Id: expr_eval_dialog.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// The initial intent for the ExprEval dialog was to display the
// contents of a variable in the program; it has evolved into a UI
// front-end for the built-in expression interpreter.
// It pops up when either "Evaluate" is selected from the right-click
// menu over the code listing area, or when Tools-->Evaluate is selected
// from the main menu
//
#include <signal.h>
#include <sstream>
#include <stack>
#include <gdk/gdkkeysyms.h>
#include "gtkmm/box.h"
#include "gtkmm/button.h"
#include "gtkmm/connect.h"
#include "gtkmm/entry.h"
#include "gtkmm/flags.h"
#include "gtkmm/label.h"
#include "gtkmm/main.h"
#include "gtkmm/resize.h"
#include "generic/lock.h"
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/debug_symbol_list.h"
#include "zdk/mutex.h"
#include "zdk/thread_util.h"
#include "zdk/variant.h"
#include "zdk/zero.h"
#include "cursor.h"
#include "eval_events.h"
#include "expr_eval_dialog.h"
#include "expr_eval_view.h"
#include "set_cursor.h"
#include "slot_macros.h"
#include "text_entry.h"

using namespace std;

static const char DIALOG_TITLE[] =  "Evaluate";



ExprEvalDialog::ExprEvalDialog(Debugger& dbg, Thread& thread)
    : Base(btn_close, DIALOG_TITLE, thread)
    , thread_(&thread)
    , entry_(0)
    , view_(manage(new ExprEvalView(dbg)))
    , btnEval_(0)
{
    dialog_closed_event.connect(
        Gtk_SLOT(static_cast<EvalEvents*>(events()),
        &EvalEvents::deactivate));

    events()->signal_deactivate()->connect(
        Gtk_BIND(Gtk_SLOT(this, &ExprEvalDialog::popdown), 0));

    Gtk::HBox* box = manage(new Gtk::HBox);
    get_vbox()->pack_start(*box, false, false);

    box->set_border_width(3);
    box->set_spacing(2);

    Gtk::Label* label = manage(new Gtk::Label("Expression: "));
    box->pack_start(*label, false, false);

    Properties* prop = CHKPTR(thread.debugger())->properties();
    entry_ = manage(new TextEntry(prop, "evaluate"));

    box->pack_start(*entry_, true, true);
    Gtk_CONNECT_0(entry_, activate, this, &ExprEvalDialog::do_it);

    btnEval_ = manage(new Gtk::Button(" _Evaluate "));
    box->pack_start(*btnEval_, false, false);

    add_button_accelerator(*btnEval_);

    Gtk_CONNECT_1(btnEval_, clicked, this, &ExprEvalDialog::on_evaluate, true);

    view_ = manage(new ExprEvalView(dbg));
    get_vbox()->pack_start(*view_);
    Gtk_set_size(view_, 550, 200);

    view_->column(0).set_width(200);
    view_->column(1).set_width(200);
    view_->show_raw_memory.connect(Gtk_SLOT(this, &ExprEvalDialog::popdown));

    // re-evaluate expression when the numeric base changes
    view_->numeric_base_changed.connect(
        Gtk_BIND(Gtk_SLOT(this, &ExprEvalDialog::on_evaluate), false));

    Gtk_set_resizable(this, true);
}


ExprEvalDialog::~ExprEvalDialog()
{
}


void ExprEvalDialog::run(Gtk::Widget* parent)
{
    if (!entry_->get_text().empty())
    {
        do_it();
    }
    else if (!view()->debug_symbols().empty())
    {
        RefPtr<DebugSymbol> sym = view()->debug_symbols().front();
        entry_->set_text(CHKPTR(sym->name())->c_str());

        do_it();
    }
    entry_->get_parent()->grab_focus();
    entry_->grab_focus();

    DialogBox::run(parent);
}


BEGIN_SLOT_(void, ExprEvalDialog::do_it,())
{
    on_evaluate(true);
}
END_SLOT()


BEGIN_SLOT(ExprEvalDialog::on_evaluate,(bool haveNewExpression))
{
    const string expr = entry_->get_text();
    if (!expr.empty())
    {
        CHKPTR(btnEval_)->set_sensitive(false);
        set_cursor(*this, Gdk_FLAG(WATCH));

        assert(thread_);
        assert(events());

        view()->reset(!haveNewExpression);
        events()->activate();

        const addr_t addr = 0;
        evaluate.emit(expr, addr, events(), view()->numeric_base());
    }
}
END_SLOT()


void ExprEvalDialog::popdown(addr_t)
{
    Gtk::Main::quit();
}


void ExprEvalDialog::update_state_eval_complete(bool)
{
    CHKPTR(view_)->display();
    CHKPTR(btnEval_)->set_sensitive(true);
    set_cursor(*this, Gdk_FLAG(TOP_LEFT_ARROW));
}


/**
 * @note main thread
 */
void ExprEvalDialog::on_done(const Variant& var)
{
    if (DebugSymbol* sym = var.debug_symbol())
    {
        view_->notify(sym);
        refresh.emit(view_);
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
