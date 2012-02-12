//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "controller.h"
#include "flvar_view.h"
#include "var_view.h"


ui::VarView::VarView(ui::Controller& c) : View(c)
{
}


ui::VarView::~VarView() throw()
{
}


void ui::VarView::update(const ui::State&)
{
    symbols_.clear();
}


bool ui::VarView::is_expanding(DebugSymbol* sym) const
{
    return expands_.count(SymKey(*sym)) != 0;
}


void ui::VarView::expand(size_t row, bool toggle)
{
    SymKey key(get_symbol(row));

    if (toggle)
    {
        expands_.insert(key);
    }
    else
    {
        expands_.erase(key);
    }
    controller().interrupt_main_thread();
}


bool ui::VarView::notify(DebugSymbol* sym)
{
    if (sym)
    {
        symbols_.push_back(sym);

        if (is_expanding(sym))
        {
            sym->read(this);
            sym->enum_children(this);
        }
    }
    return true;
}


DebugSymbol& ui::VarView::get_symbol(size_t n) const
{
    if (n >= symbols_.size())
    {
        throw std::out_of_range(__func__);
    }

    return *symbols_[n];
}


