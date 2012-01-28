#ifndef QUICK_VIEW_DIALOG_H__5E5A924C_8E4B_4EF1_A8AE_8A66D30A786F
#define QUICK_VIEW_DIALOG_H__5E5A924C_8E4B_4EF1_A8AE_8A66D30A786F
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
#include "dialog_box.h"
#include "eval_base.h"

class Debugger;
class DebugSymbol;
class EvalEvents;
class VariablesView;
class TextEntry;


/**
 * A dialog for viewing variables and evaluating expressions
 */
CLASS ExprEvalDialog : public EvalBase<ExprEvalDialog, DialogBox>
{
public:
    ExprEvalDialog(Debugger&, Thread&);
    ~ExprEvalDialog();

    VariablesView* view() { return view_; }

    void run(Gtk::Widget* = 0);

    void update_state_eval_complete(bool displayResults = false);

    void popdown(addr_t = 0);

    TextEntry* text_entry() { return entry_; }

    void on_done(const Variant& var);

protected:
    void do_it();

    void on_evaluate(bool haveNewExpr); // "Evaluate" button clicked

private:
    RefPtr<Thread>      thread_;
    TextEntry*          entry_;
    VariablesView*      view_;
    Gtk::Button*        btnEval_;
};


#endif // QUICK_VIEW_DIALOG_H__5E5A924C_8E4B_4EF1_A8AE_8A66D30A786F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
