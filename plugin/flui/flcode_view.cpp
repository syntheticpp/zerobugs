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


using namespace std;


static const int LABEL_HEIGHT = 25;
static const string arrow = { "arrow" };


FlCodeView::FlCodeView(

    ui::Controller& controller,
    const char*     filename)

    : base_type(controller, 0, 0, 0, 0)
{
    widget()->set_mark_pixmap(arrow, arrow_xpm);
    widget()->copy_label(filename);
    widget()->labelfont(FL_TIMES);
}


FlCodeView::~FlCodeView() throw()
{
}


void FlCodeView::update(const ui::State& s)
{
    if (RefPtr<Symbol> sym = s.current_symbol())
    {
        assert(sym->line());

        widget()->read_file(sym->file()->c_str());
        widget()->set_mark_at_line(widget()->highlighted_line(), arrow, false);
        widget()->highlight_line(sym->line());
        widget()->set_mark_at_line(sym->line(), arrow);
    }
}


////////////////////////////////////////////////////////////////
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


void FlMultiCodeView::update(const ui::State& s)
{
    ui::MultiCodeView::update(s);
}


RefPtr<ui::CodeView> FlMultiCodeView::make_view(const Symbol& sym)
{
    RefPtr<ui::CodeView> view;

    if (sym.line() == 0) // no source code?
    {
        // TODO: make an assembly code view
        assert(false);
    }
    else
    {
        RefPtr<FlCodeView> codeView = new FlCodeView(
            controller(),
            basename(sym.file()->c_str()));

        view = codeView;
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

