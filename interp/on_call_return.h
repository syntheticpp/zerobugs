#ifndef ON_CALL_RETURN_H__9C154A0D_66B8_4240_A472_6E9111C4681E
#define ON_CALL_RETURN_H__9C154A0D_66B8_4240_A472_6E9111C4681E
//
// $Id: on_call_return.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/zero.h"
#include "cpu_state_saver.h"
#include "expr.h"
#include "interp.h"

class CallSetup;

/**
 * Breakpoint action invoked upon returning from a function
 * call triggered by the expression interpreter.
 * Purposes:
 * set the return value as the result of the corresponding
 * sub-expression;
 * and break in the debugger if there are no more sub-expressions
 * pending evaluation (i.e. finished evaluating the entire expr).
 */
CLASS OnCallReturn : public ZObjectImpl<BreakPoint::Action>
                   , private CPUStateSaver
                   , private DebugSymbolEvents
{
public:
BEGIN_INTERFACE_MAP(OnCallReturn)
    INTERFACE_ENTRY(BreakPoint::Action)

END_INTERFACE_MAP()

    /**
     * @param expr sub-expression whose evaluation resulted
     * in a function call
     * @param set_result pointer to method that sets the
     * result of the sub-expression
     * @param func symbol table entry (not debug symbol)
     * that corresponds to the called function
     * @param thread the thread on which the function is called
     */
    OnCallReturn (
        RefPtr<Expr> expr,
        RefPtr<Symbol> func,
        const CallSetup&,
        Thread& thread,
        DebugInfoReader*
	);

    virtual ~OnCallReturn() throw();

private:
    RefPtr<DebugSymbol> ret_symbol(Thread*);
    bool compute_ret_value(Thread*, RefPtr<DebugSymbol>);

    /**** BreakPoint::Action *****/
    virtual const char* name() const;

    virtual word_t cookie() const;

    virtual bool execute(Thread* thread, BreakPoint* bpnt);

    /**** DebugSymbolEvents *****/
    bool notify(DebugSymbol*) { return false; }

    bool is_expanding(DebugSymbol*) const { return false; }

    int numeric_base(const DebugSymbol* sym) const
    { return interp_->numeric_base(); }

    void symbol_change(DebugSymbol*, DebugSymbol*) { };

private:
    RefPtr<Expr>        expr_;
    RefPtr<Interp>      interp_;
    RefPtr<Symbol>      func_;
    DebugInfoReader*    debugInfo_;
    addr_t              retAddr_;
    WeakPtr<DataType>   retType_;
    size_t              frame_;
    reg_t               pc_;
    RefPtr<BreakPointAction> oldAct_;
};

#endif // ON_CALL_RETURN_H__9C154A0D_66B8_4240_A472_6E9111C4681E
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
