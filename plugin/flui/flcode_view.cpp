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
#include <FL/Enumerations.H>

using namespace std;


static const int LABEL_HEIGHT = 25;


FlSourceView::FlSourceView(

    ui::Controller& controller,
    const char*     filename)

    : base_type(controller, 0, 0, 0, 0)
{
    widget()->set_listing(this);
    widget()->set_mark_pixmap(Fl_CodeTable::mark_arrow, arrow_xpm);
    widget()->copy_label(filename);
}


FlSourceView::~FlSourceView() throw()
{
}


void FlSourceView::show(RefPtr<Thread> t, RefPtr<Symbol> sym)
{
    widget()->refresh(t, sym);
}


void FlSourceView::update(const ui::State& s)
{
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


void FlAsmView::update(const ui::State& state)
{
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

