//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/symbol.h"
#include "flcallback.h"
#include "flcode_view.h"
#include "flcode_table.h"
#include "flpack_layout.h"
#include "icons/arrow.xpm"
#include <FL/Enumerations.H>

#include <iostream>

using namespace std;


static const int LABEL_HEIGHT = 25;
static const string arrow = { "arrow" };


FlSourceView::FlSourceView(

    ui::Controller& controller,
    const char*     filename)

    : base_type(controller, 0, 0, 0, 0)
{
    widget()->set_mark_pixmap(arrow, arrow_xpm);
    widget()->copy_label(filename);
}


FlSourceView::~FlSourceView() throw()
{
}


void FlSourceView::show(RefPtr<Symbol> sym)
{
    const size_t line = sym->line();
    assert(line);

    // @note: the widget is expected to deal with duplicate reads
    widget()->read_file(sym->file()->c_str());

    widget()->set_mark_at_line(widget()->highlighted_line(), arrow, false);
    widget()->highlight_line(line);
    widget()->set_mark_at_line(line, arrow);

    widget()->show();
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
}


FlAsmView::~FlAsmView() throw()
{
}


void FlAsmView::update(const ui::State& state)
{
}


void FlAsmView::show(RefPtr<Symbol> sym)
{
#if DEBUG
    std::clog << __func__ << ": " << sym->name()->c_str() << std::endl;
#endif

    widget()->show();
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


void FlMultiCodeView::show(RefPtr<Symbol>)
{
    // nothing to be seen here, move on...
}


void FlMultiCodeView::update(const ui::State& s)
{
    ui::MultiCodeView::update(s);
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
    if (auto v = dynamic_cast<FlViewBase*>(view.get()))
    {
        widget()->value(v->base_widget());
    }
}

