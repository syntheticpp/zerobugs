//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "zdk/breakpoint.h"
#include "command.h"
#include "controller.h"
#include "flbreakpoint_view.h"

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

    table->set_event_callback([table, this] {

        if (Fl::event_button1() && Fl::event_is_click())
        {
            const size_t row = table->callback_row();
            const size_t col = table->callback_col();

            if (row < this->size())
            {
                const addr_t addr = (*this)[row].bpoint->addr();
                if (col == Fl_BreakPointTable::COL_Pixmap)
                {
                    ui::call_main_thread(controller(), [this, addr] {
                        controller().enable_user_breakpoint(addr, ui::Toggle);
                    });
                }
                else if (col == Fl_BreakPointTable::COL_Condition)
                {
                    controller().show_edit_breakpoint_dialog(addr);
                }
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

