#ifndef FLEVAL_DLG_H__028B8801_3B9B_4EF1_AB62_A48E1EA66547
#define FLEVAL_DLG_H__028B8801_3B9B_4EF1_AB62_A48E1EA66547
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fldialog.h"
#include "zdk/ref_ptr.h"

class ExprEvents;   // zdk/expr.h"
typedef RefPtr<ExprEvents> ExprEventsPtr;

class Fl_Input;
class FlVarView;


////////////////////////////////////////////////////////////////

class FlEvalDialog : public FlDialog
{
public:
    explicit FlEvalDialog(ui::Controller&);
    ~FlEvalDialog();

private:
    static void eval_callback(Fl_Widget*, void*);

    void close();
    void eval();
    void update(const ui::State&);

private:
    ExprEventsPtr       events_;
    Fl_Input*           input_;
    RefPtr<Thread>      thread_;

    RefPtr<FlVarView>   view_;
};


#endif // FLEVAL_DLG_H__028B8801_3B9B_4EF1_AB62_A48E1EA66547

