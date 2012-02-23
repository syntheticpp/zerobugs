//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/zero.h"
#include "flcallback.h"
#include "flcode_view.h"
#include "flcode_table.h"
#include "flpack_layout.h"
#include <FL/Enumerations.H>
#include <iostream>

using namespace std;

static const int LABEL_HEIGHT = 25;


FlSourceView::FlSourceView(

    ui::Controller& controller,
    const char*     filename)

    : base_type(controller, 0, 0, 0, 0)
{
    widget()->copy_label(filename);
    widget()->set_listing(this);
}


FlSourceView::~FlSourceView() throw()
{
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

    SharedStringPtr mark = bpnt.is_enabled()
        ? Fl_CodeTable::mark_stop_enabled
        : Fl_CodeTable::mark_stop_disabled;

    widget()->set_mark_at_line(line, mark);
}


////////////////////////////////////////////////////////////////
FlAsmView::FlAsmView(ui::Controller& controller, const Symbol& sym)
    : base_type(controller, 0, 0, 0, 0)
{
    const char* fname = basename(sym.file()->c_str());
    
    widget()->copy_label(fname);
    widget()->set_listing(this);
}


FlAsmView::~FlAsmView() throw()
{
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

    widget()->set_mark_at_line(addr_to_line(addr), mark);
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


ui::Layout::CallbackPtr FlMultiCodeView::make_callback()
{
    return std::unique_ptr<ui::LayoutCallback>(
        new Callback(
            widget(),
            widget()->x(),
            widget()->y() + LABEL_HEIGHT,
            widget()->w(),
            widget()->h() - LABEL_HEIGHT));
}


void FlMultiCodeView::make_visible(RefPtr<CodeView> view)
{
    widget()->value(dynamic_cast<FlViewBase&>(*view).base_widget());
}

