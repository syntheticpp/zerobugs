#ifndef FLBREAKPOINT_VIEW_H__8E45ACAC_69C4_44C0_A1F2_D54D7BF0EB50
#define FLBREAKPOINT_VIEW_H__8E45ACAC_69C4_44C0_A1F2_D54D7BF0EB50
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "breakpoint_view.h"
#include "flbreakpoint_table.h"
#include "flview.h"


class FlBreakPointView : public FlView<ui::BreakPointView, FlBreakPointTable>
{
public:
    FlBreakPointView(ui::Controller&, int x = 0, int y = 0, int w = 0, int h = 0);

protected:
    virtual ~FlBreakPointView() throw();

    virtual void update_breakpoint(BreakPoint&);
};


#endif // FLBREAKPOINT_VIEW_H__8E45ACAC_69C4_44C0_A1F2_D54D7BF0EB50

