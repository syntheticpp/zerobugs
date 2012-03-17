//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/zero.h"
#include "zdk/breakpoint_util.h"
#include "command.h"
#include "const.h"
#include "controller.h"
#include "flcallback.h"
#include "flcode_view.h"
#include "flcode_table.h"
#include "flpack_layout.h"
#include <FL/Enumerations.H>

using namespace std;


static void set_event_callback(

    ui::CodeView*   view,
    Fl_CodeTable*   widget )
{
    widget->set_event_callback([view](Fl_CodeTable& w) {

        if (Fl::event_button1() && Fl::event_is_click())
        {
            w.select_callback_row();

            if (w.callback_col() == Fl_CodeTable::COL_Mark)
            {
                auto& controller = view->controller();
                ui::call_main_thread(controller,[&controller](){
                    controller.toggle_breakpoint();
                });
            }

            if (auto parent = view->parent())
            {
                parent->set_current_view(view);
            }
        }
    });
}


FlSourceView::FlSourceView(

    ui::Controller& controller,
    const char*     filename)

    : base_type(controller, 0, 0, 0, 0)
{
    widget()->copy_label(filename);
    widget()->set_listing(this);
    set_event_callback(this, widget());
}


FlSourceView::~FlSourceView() throw()
{
}


int FlSourceView::selected_line() const
{
    return widget()->selected_row() + 1;
}


void FlSourceView::show(RefPtr<Thread> t, RefPtr<Symbol> sym)
{
    widget()->refresh(t, sym);
}


void FlSourceView::update(const ui::State&)
{
    widget()->remove_all_marks(Fl_CodeTable::mark_stop_enabled);
    widget()->remove_all_marks(Fl_CodeTable::mark_stop_disabled);
}


void FlSourceView::update_breakpoint(BreakPoint& bpnt)
{
    const size_t line = bpnt.symbol()->line();

    SharedStringPtr mark = has_enabled_actions(bpnt)
        ? Fl_CodeTable::mark_stop_enabled
        : Fl_CodeTable::mark_stop_disabled;

    widget()->set_mark_at_line(line, mark);
}


////////////////////////////////////////////////////////////////
FlAsmView::FlAsmView(

    ui::Controller& controller,
    const Symbol&   sym )

    : base_type(controller, 0, 0, 0, 0)
{
    const char* fname = basename(sym.file()->c_str());

    widget()->copy_label(fname);
    widget()->set_listing(this);
    set_event_callback(this, widget());
}


FlAsmView::~FlAsmView() throw()
{
}


int FlAsmView::selected_line() const
{
    return widget()->selected_row() + 1;
}


void FlAsmView::update(const ui::State&)
{
    widget()->remove_all_marks(Fl_CodeTable::mark_stop_enabled);
    widget()->remove_all_marks(Fl_CodeTable::mark_stop_disabled);
}


void FlAsmView::update_breakpoint(BreakPoint& bpnt)
{
    addr_t addr = bpnt.symbol()->addr();
    if (bpnt.is_deferred())
    {
        // TODO: adjust address
    }

    SharedStringPtr mark = bpnt.is_enabled()
        ? Fl_CodeTable::mark_stop_enabled
        : Fl_CodeTable::mark_stop_disabled;

    try
    {
        widget()->set_mark_at_line(addr_to_line(addr), mark);
    }
    catch (const out_of_range&)
    {
        // does not fit in view, ok
    }
}


void FlAsmView::show(RefPtr<Thread> t, RefPtr<Symbol> sym)
{
    widget()->refresh(t, sym);
}


////////////////////////////////////////////////////////////////
//
// multi view
//
FlMultiCodeView::FlMultiCodeView(ui::Controller& controller)
    : base_type(controller, 0, 0, 0, 0)
{
    widget()->end();
}


FlMultiCodeView::~FlMultiCodeView() throw ()
{
}


void FlMultiCodeView::clear()
{
    widget()->clear();
    MultiCodeView::clear();
}


void FlMultiCodeView::show(RefPtr<Thread>, RefPtr<Symbol>)
{
}


ui::Layout::CallbackPtr FlMultiCodeView::make_callback()
{
    return std::unique_ptr<ui::LayoutCallback>(
        new Callback(
            widget(),
            widget()->x(),
            widget()->y() + ui::Const::label_height,
            widget()->w(),
            widget()->h() - ui::Const::label_height - 1));
}


RefPtr<ui::CodeView> FlMultiCodeView::make_view(const Symbol& sym)
{
    RefPtr<ui::CodeView> view;

    if (sym.line() == 0) // no source code?
    {
        view = new FlAsmView(controller(), sym);
    }
    else
    {
        const char* fname = basename(sym.file()->c_str());
        view = new FlSourceView(controller(), fname);
    }
    return view;
}


void FlMultiCodeView::make_visible(RefPtr<CodeView> view)
{
    if (view)
    {
        widget()->value(dynamic_cast<FlViewBase&>(*view).base_widget());
    }
}

