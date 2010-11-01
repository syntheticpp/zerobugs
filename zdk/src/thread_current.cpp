// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: thread_current.cpp 714 2010-10-17 10:03:52Z root $
//
#include "zdk/thread_util.h"
#include "zdk/zobject_scope.h"


Frame* thread_current_frame(const Thread* thread)
{
    Frame* result = NULL;
    if (thread)
    {
        StackTrace* stack = thread->stack_trace();
        if (stack && stack->size())
        {
            result = stack->selection();
        }
    }
    return result;
}


/**
 * @return the function at the currently selected frame, or NULL
 */
Symbol* thread_current_function(const Thread* thread)
{
    Symbol* result = NULL;
    if (Frame* frame = thread_current_frame(thread))
    {
        result = frame->function();
    }
    return result;
}


/**
 * @return the symbol table at the currently selected frame, or NULL
 */
RefPtr<SymbolTable> thread_current_symbol_table(const Thread* thread)
{
    RefPtr<SymbolTable> table;
    if (Symbol* fun = thread_current_function(thread))
    {
        ZObjectScope scope;
        table = fun->table(&scope);
    }
    return table;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
