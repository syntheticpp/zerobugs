//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
//
#include "expr_events.h"
#include "controller.h"
#include "dialog.h"


ui::ExprEvalEvents::~ExprEvalEvents() throw()
{
}

/**
 * An error occurred while interpreting expression
 */
void ui::ExprEvalEvents::on_error(const char* msg)
{
    controller_.status_message(msg);
}

void ui::ExprEvalEvents::on_warning(const char* msg)
{
    controller_.status_message(msg);
}

/**
 * An event occurred on thread while interpreting expression
 * (i.e. a signal was raised, and it was not purposely caused
 * by the interpreter).
 * @return true if handled
 */
bool ui::ExprEvalEvents::on_event(Thread*, Platform::addr_t)
{
    return true;
}

/**
 * The interpreter calls a function inside the debugged program.
 * @param retAddr return address of function
 * @param symbol if not NULL, the interpreter is about to call
 * the function of the corresponding symbol table entry; if the
 * symbol is NULL, the function has returned.
 */
void ui::ExprEvalEvents::on_call(

    Platform::addr_t    retAddr,
    Symbol*             sym /* = nullptr */)
{
}

