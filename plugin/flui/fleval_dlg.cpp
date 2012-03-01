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
#include "zdk/expr.h"
#include "zdk/observer_impl.h"
#include "zdk/variant.h"
#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <iostream>
using namespace std;


////////////////////////////////////////////////////////////////

class Events : public SubjectImpl<ExprEvents>
{
public:
    explicit Events(RefPtr<FlVarView> v) : view_(v)
    { }

private:
    BEGIN_INTERFACE_MAP(Events)
        INTERFACE_ENTRY_INHERIT(SubjectImpl<ExprEvents>)
    END_INTERFACE_MAP()

    Events(const Events& other)
        : view_(other.view_) {
    }

    /**
     * Indicates that the interpreter has finished parsing and
     * evaluating. When this notification is received, it is up
     * to the implementation whether to continue silently, or
     * enter interactive mode (prompt).
     * @return true to enter interactive mode
     */
    virtual bool on_done(
        Variant*            var,
        bool*               interactive,
        DebugSymbolEvents*  cb = nullptr)
     {
        if (var)
        {
            if (auto sym = var->debug_symbol())
            {
                view_->notify(sym);
                const int n = view_->variable_count();
                view_->widget()->rows(n);
                view_->widget()->redraw();

            #if DEBUG
                clog << __func__ << ": " << n << " vars" << endl;
                if (sym->value())
                {
                    clog << sym->value()->c_str() << endl;
                }
            #endif
            }
        }
        return true;
     }

    /**
     * An error occurred while interpreting expression
     */
    virtual void on_error(const char* msg) {
    #if DEBUG
        clog << __func__ << ": " << msg << endl;
    #endif
    }

    virtual void on_warning(const char* msg) {
    #if DEBUG
        clog << __func__ << ": " << msg << endl;
    #endif
    }

    /**
     * An event occurred on thread while interpreting expression
     * (i.e. a signal was raised, and it was not purposely caused
     * by the interpreter).
     * @return true if handled
     */
    virtual bool on_event(Thread*, Platform::addr_t) {
    #if DEBUG
        clog << __func__ << endl;
    #endif
        return true;
    }

    /**
     * The interpreter calls a function inside the debugged program.
     * @param retAddr return address of function
     * @param symbol if not NULL, the interpreter is about to call
     * the function of the corresponding symbol table entry; if the
     * symbol is NULL, the function has returned.
     */
    virtual void on_call(Platform::addr_t retAddr, Symbol* = nullptr) {
    #if DEBUG
        clog << __func__ << endl;
    #endif
    }

    virtual ExprEvents* clone() const {
        return new Events(*this);
    }

private:
    RefPtr<FlVarView> view_;
};


////////////////////////////////////////////////////////////////
FlEvalDialog::FlEvalDialog(ui::Controller& c)
    : FlDialog(c, 0, 0, 500, 300, "Evaluate Expression")
    , input_(nullptr)
{
    auto g = new Fl_Group(0, 0, 500, 300);
    g->box(FL_EMBOSSED_FRAME);
    input_ = new Fl_Input(20, 20, 500 - 150, 20);
    auto okBtn = new Fl_Button(500 - 150 + 20 + 10, 20 - 3, 100, 25, "&Evaluate");
    okBtn->callback(eval_callback, this);
    view_ = new FlVarView(c);
    g->end();

    view_->resize(20, 55, 500 - 40, 300 - 80);
    add_view(view_);

    center();
}


FlEvalDialog::~FlEvalDialog()
{
}


void FlEvalDialog::close()
{
    view_->clear(true);
    controller().set_current_dialog(nullptr);
    hide();
}


void FlEvalDialog::eval_callback(Fl_Widget* w, void* data)
{
    reinterpret_cast<FlEvalDialog*>(data)->eval();
}


void FlEvalDialog::eval()
{
    string expr = input_->value();
    if (expr.empty())
    {
        return;
    }
    view_->clear(true);
    RefPtr<Events> events(new Events(view_));

    auto* d = controller().debugger();

    ui::call_main_thread(controller(), [d, expr, thread_, events]() {
        d->evaluate(expr.c_str(), thread_.get(), 0, events.get());
    });
}


void FlEvalDialog::update(const ui::State& s)
{
    ui::Dialog::update(s);
    thread_ = s.current_thread();
}

