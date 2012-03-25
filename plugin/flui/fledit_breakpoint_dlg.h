#ifndef FLEDIT_BREAKPOINT_DLG_H__4905B144_B0DD_4E21_AC98_BA87E487FD03
#define FLEDIT_BREAKPOINT_DLG_H__4905B144_B0DD_4E21_AC98_BA87E487FD03
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fledit_breakpoint_dlg.h"
#include "fldialog.h"


class FlEditBreakPointDlg : public FlDialog
{
public:
    explicit FlEditBreakPointDlg(ui::Controller&);

protected:
    void close();
};

#endif // FLEDIT_BREAKPOINT_DLG_H__4905B144_B0DD_4E21_AC98_BA87E487FD03

