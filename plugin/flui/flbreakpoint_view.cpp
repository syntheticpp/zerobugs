//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/breakpoint.h"
#include "command.h"
#include "controller.h"
#include "flbreakpoint_view.h"
#include <iostream>

using namespace std;


FlBreakPointView::FlBreakPointView(

    ui::Controller& c,
    int             x,
    int             y,
    int             w,
    int             h)

: base_type(c, this, x, y, w, h, "Breakpoints")
{
    auto table = widget();

    table->set_event_callback([table, this]() {
        if (Fl::event_button1() && Fl::event_is_click())
        {
            size_t row = table->callback_row();
            size_t col = table->callback_col();

            if (row < this->size() && col == 0)
            {
                addr_t addr = (*this)[row]->addr();

                auto& controller = this->controller();
                ui::call_main_thread(controller, [&controller, addr](){
                    controller.enable_user_breakpoint(addr, ui::Toggle);
                });
            }
        }
    });
}


FlBreakPointView::~FlBreakPointView() throw()
{
}


void FlBreakPointView::update_breakpoint(BreakPoint& bp)
{
    base_type::update_breakpoint(bp);
    widget()->rows(size());
}

