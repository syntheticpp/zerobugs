#ifndef FLEVAL_DLG_H__028B8801_3B9B_4EF1_AB62_A48E1EA66547
#define FLEVAL_DLG_H__028B8801_3B9B_4EF1_AB62_A48E1EA66547
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
#include "fldialog.h"
#include "zdk/debug_sym.h"
#include "zdk/ref_ptr.h"

class ExprEvents;   // zdk/expr.h"
typedef RefPtr<ExprEvents> ExprEventsPtr;

class Fl_Input;
class Fl_Output;
class FlVarView;


/**
 * Dialog that allows the user to enter a C/C++
 * expression and evaluate it.
 */
class FlEvalDialog : public FlDialog, DebugSymbolEvents
{
public:
    explicit FlEvalDialog(ui::Controller&);
    ~FlEvalDialog();

private:
    // query_interface implementation
    BEGIN_INTERFACE_MAP(FlEvalDialog)
        INTERFACE_ENTRY(DebugSymbolEvents)
    END_INTERFACE_MAP()

    static void eval_callback(Fl_Widget*, void*);

    void clear();
    void close();
    void eval();

    bool status_message(const char* msg);

    void update(const ui::State&);

    // --- DebugSymbolEvents interface
    /**
     * Notified when a new symbol (possibly the child of an
     * aggregated object such a class instance or array) is
     * detected; if the method returns true, the symbol's
     * value is read.
     */
    virtual bool notify(DebugSymbol*);

    /**
     * Symbols that correspond to aggregate objects such as
     * class instances or arrays may be expanded, so that the
     * user can inspect their sub-parts. This method is called
     * by the reader implementations to determine if the client
     * wants such an aggregate object to be expanded or not.
     */
    virtual bool is_expanding(DebugSymbol*) const {
        return false;
    }

    /**
     * Readers call this method to determine what numeric base
     * should be used for the representation of integer values.
     */
    virtual int numeric_base(const DebugSymbol*) const {
        return 0;
    }

    /**
     * A change in the symbol has occurred (name, type, address
     * etc.) A pointer to the old values is passed in.
     */
    virtual void symbol_change(
        DebugSymbol* /* newSym */,
        DebugSymbol* /* oldSym */) {
    }

private:
    typedef std::vector<RefPtr<DebugSymbol> > Variables;

    ExprEventsPtr       events_;
    Fl_Input*           input_;
    Fl_Output*          status_;

    // thread in the context of which expressions are evaluated
    RefPtr<Thread>      thread_;

    Variables           vars_;
    RefPtr<FlVarView>   view_;
};


#endif // FLEVAL_DLG_H__028B8801_3B9B_4EF1_AB62_A48E1EA66547

