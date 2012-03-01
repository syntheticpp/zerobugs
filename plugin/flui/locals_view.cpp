//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "controller.h"
#include "flvar_view.h"
#include "locals_view.h"


ui::LocalsView::LocalsView(ui::Controller& c)
    : VarView(c)
{
}


void ui::LocalsView::update(const ui::State& state)
{
    clear();
#if 0
    VarView::update(state);
#else
    update_scope(state);
#endif

    if (auto t = state.current_thread())
    {
        controller().debugger()->enum_variables(
            t.get(),
            nullptr,    // name
            nullptr,    // function
            this,
            LOOKUP_LOCAL);
    }
}

