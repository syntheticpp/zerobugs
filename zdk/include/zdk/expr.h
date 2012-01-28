#ifndef EXPR_H__BA0431C0_E88E_49D5_B643_54DE37EADD51
#define EXPR_H__BA0431C0_E88E_49D5_B643_54DE37EADD51
//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/platform.h"
#include "zdk/observer.h"
#include "zdk/zerofwd.h"



/**
 * Observer that receives asynchronous notifications from the
 * expression interpreter.
 */
DECLARE_ZDK_INTERFACE_(ExprEvents, Subject)
{
    DECLARE_UUID("1f16796e-0aed-4d98-b35a-ffb907a4db15")

    /**
     * Indicates that the interpreter has finished parsing and
     * evaluating. When this notification is received, it is up
     * to the implementation whether to continue silently, or
     * enter interactive mode (prompt).
     * @return true to enter interactive mode
     */
    virtual bool on_done(Variant*,
                         bool* interactive,
                         DebugSymbolEvents* = 0) = 0;

    /**
     * An error occurred while interpreting expression
     */
    virtual void on_error(const char*) = 0;

    virtual void on_warning(const char*) = 0;

    /**
     * An event occurred on thread while interpreting expression
     * (i.e. a signal was raised, and it was not purposely caused
     * by the interpreter).
     * @return true if handled
     */
    virtual bool on_event(Thread*, Platform::addr_t) = 0;

    /**
     * The interpreter calls a function inside the debugged program.
     * @param retAddr return address of function
     * @param symbol if not NULL, the interpreter is about to call
     * the function of the corresponding symbol table entry; if the
     * symbol is NULL, the function has returned.
     */
    virtual void on_call(Platform::addr_t retAddr, Symbol* = 0) = 0;

    virtual ExprEvents* clone() const = 0;
};

#endif // EXPR_H__BA0431C0_E88E_49D5_B643_54DE37EADD51
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
