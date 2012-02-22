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
#include "icons/arrow.xpm"
#include "icons/stop_red.xpm"
#include "icons/stop_pink.xpm"
#include <FL/Enumerations.H>

using namespace std;


static const int LABEL_HEIGHT = 25;


FlSourceView::FlSourceView(

    ui::Controller& controller,
    const char*     filename)

    : base_type(controller, 0, 0, 0, 0)
{
    widget()->copy_label(filename);
    widget()->set_listing(this);

    widget()->set_mark_pixmap(Fl_CodeTable::mark_arrow, arrow_xpm);
    widget()->set_mark_pixmap(Fl_CodeTable::mark_stop_enabled, stop_red_xpm);
    widget()->set_mark_pixmap(Fl_CodeTable::mark_stop_disabled, stop_pink_xpm);
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
    widget()->set_mark_at_line(-1, Fl_CodeTable::mark_stop_enabled, false);
    widget()->set_mark_at_line(-1, Fl_CodeTable::mark_stop_disabled, false);
}


void FlSourceView::update_breakpoint(BreakPoint& bpnt)
{
    size_t line = bpnt.symbol()->line();

    if (bpnt.is_enabled())
    {
        widget()->set_mark_at_line(line, Fl_CodeTable::mark_stop_enabled);
    }
    else
    {
        widget()->set_mark_at_line(line, Fl_CodeTable::mark_stop_disabled);
    }
}


////////////////////////////////////////////////////////////////
FlAsmView::FlAsmView(ui::Controller& controller, const Symbol& sym)
    : base_type(controller, 0, 0, 0, 0)
{
    const char* fname = basename(sym.file()->c_str());
    widget()->copy_label(fname);
    widget()->set_listing(this);
    widget()->set_mark_pixmap(Fl_CodeTable::mark_arrow, arrow_xpm);
}


FlAsmView::~FlAsmView() throw()
{
}


void FlAsmView::update(const ui::State&)
{
    // TODO: remove all breakpoint marks
}


void FlAsmView::update_breakpoint(BreakPoint& bpnt)
{
    // TODO:
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

