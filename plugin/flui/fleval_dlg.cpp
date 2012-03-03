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
#include <FL/Fl_Output.H>


////////////////////////////////////////////////////////////////
FlEvalDialog::FlEvalDialog(ui::Controller& c)
    : FlDialog(c, 0, 0, 600, 400, "Evaluate Expression")
    , input_(nullptr)
    , status_(nullptr)
{
    group()->box(FL_EMBOSSED_FRAME);
    input_ = new Fl_Input(20, 20, 450, 22);

    // todo: get font from VarView
    input_->textfont(FL_HELVETICA);
    input_->textsize(11);

    auto okBtn = new Fl_Button(480, 19, 100, 24, "&Evaluate");
    okBtn->callback(eval_callback, this);

    view_ = new FlVarView(c);
    view_->widget()->box(FL_DOWN_BOX);
    view_->widget()->end();

    status_ = new Fl_Output(20, 370, 560, 22);
    status_->box(FL_FLAT_BOX);
    status_->color(FL_BACKGROUND_COLOR);
    status_->clear_visible_focus();

    group()->resizable(view_->widget());
    group()->end();
    view_->resize(20, 55, 560, 310);

    center();
    set_resizable(420 /* min width allowed */, 200 /* min height */);
}


FlEvalDialog::~FlEvalDialog()
{
}


void FlEvalDialog::clear()
{
    view_->clear(true);
    vars_.clear();
    status_->value("");
}


void FlEvalDialog::close()
{
    clear();
    hide();
    FlDialog::close();
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
    clear();

    auto events = ui::ExprEvalEvents::make(controller(), this);

    auto* d = controller().debugger();

    ui::call_main_thread(controller(), [d, expr, thread_, events]() {
        d->evaluate(expr.c_str(), thread_.get(), 0, events.get());
    });
}


bool FlEvalDialog::status_message(const char* msg)
{
    assert(status_);
    status_->value(msg);
    return true;
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

