#ifndef FLEDIT_BREAKPOINT_DLG_H__4905B144_B0DD_4E21_AC98_BA87E487FD03
#define FLEDIT_BREAKPOINT_DLG_H__4905B144_B0DD_4E21_AC98_BA87E487FD03
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "zdk/platform.h"
#include "breakpoint_view.h"
#include "fldialog.h"

using namespace Platform;

class Fl_Int_Input;
class Fl_Input;
class Fl_Output;



class FlEditBreakPointDlg : public FlDialog
{
public:
    explicit FlEditBreakPointDlg(ui::Controller&);

    void popup(const ui::State&, ui::UserBreakPoint&);

protected:
    void update_breakpoint(BreakPoint&);

private:
    Fl_Input*           condition_;
    Fl_Output*          descr_;
    Fl_Int_Input*       hitCount_;  // NOT USED
    ui::UserBreakPoint  ubp_;
};

#endif // FLEDIT_BREAKPOINT_DLG_H__4905B144_B0DD_4E21_AC98_BA87E487FD03

