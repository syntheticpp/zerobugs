//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "code_view.h"
#include "flcode_table.h"


ui::CodeView::CodeView(ui::Controller& controller)
    : ui::View(controller)
{
}


ui::CodeView::~CodeView() throw()
{
}


void ui::CodeView::update(const ui::State&)
{
    
}


ui::MultiCodeView::MultiCodeView(ui::Controller& controller)
    : ui::CodeView(controller)
{
}


ui::MultiCodeView::~MultiCodeView() throw()
{
}


void ui::MultiCodeView::update(const ui::State& s)
{
    if (RefPtr<Symbol> sym = s.current_symbol())
    {
        SharedStringPtr filename = sym->file();

        auto i = views_.find(filename);
        if (i == views_.end())
        {
            i = views_.insert(std::make_pair(filename, make_view(*sym))).first;
        }

        i->second->update(s);
    }
}

