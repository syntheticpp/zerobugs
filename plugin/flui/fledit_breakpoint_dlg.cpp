//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fledit_breakpoint_dlg.h"


FlEditBreakPointDlg::FlEditBreakPointDlg(
    ui::Controller& controller )

: FlDialog( controller, 0, 0, 600, 350, "Edit Breakpoint" )
{
    center();
}


void FlEditBreakPointDlg::close()
{
    hide();
    FlDialog::close();
}

