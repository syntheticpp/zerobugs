//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fleval_dlg.h"
#include "flvar_view.h"
#include "command.h"
#include "controller.h"
#include "expr_events.h"
#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>


////////////////////////////////////////////////////////////////
FlEvalDialog::FlEvalDialog(ui::Controller& c)
    : FlDialog(c, 0, 0, 600, 400, "Evaluate Expression")
    , input_(nullptr)
{
    auto g = new Fl_Group(0, 0, 600, 400);
    g->box(FL_EMBOSSED_FRAME);
    input_ = new Fl_Input(20, 20, 450, 22);

    // todo: get font from VarView
    input_->textfont(FL_HELVETICA);
    input_->textsize(11);

    auto okBtn = new Fl_Button(480, 19, 100, 24, "&Evaluate");
    okBtn->callback(eval_callback, this);
    view_ = new FlVarView(c);
    g->end();

    view_->resize(20, 55, 560, 320);
    add_view(view_);

    center();
}


FlEvalDialog::~FlEvalDialog()
{
}


void FlEvalDialog::close()
{
    view_->clear(true);
    vars_.clear();

    controller().set_current_dialog(nullptr);
    show(false); // hide
}


void FlEvalDialog::eval_callback(Fl_Widget* w, void* data)
{
    reinterpret_cast<FlEvalDialog*>(data)->eval();
}


void FlEvalDialog::eval()
{
    std::string expr = input_->value();
    if (expr.empty())
    {
        return;
    }
    view_->clear(true);
    vars_.clear();

    RefPtr<ui::ExprEvalEvents> events = new ui::ExprEvalEvents(controller(), this);

    auto* d = controller().debugger();

    ui::call_main_thread(controller(), [d, expr, thread_, events]() {
        d->evaluate(expr.c_str(), thread_.get(), 0, events.get());
    });
}


void FlEvalDialog::update(const ui::State& s)
{
    ui::Dialog::update(s);
    thread_ = s.current_thread();

    view_->clear();
    for (auto v = begin(vars_); v != end(vars_); ++v)
    {
        view_->notify(v->get());
    }
    view_->widget()->rows(view_->variable_count());
    view_->widget()->redraw();
}


bool FlEvalDialog::notify(DebugSymbol* sym)
{
    if (sym)
    {
        vars_.push_back(sym);
    }
    return true;
}

