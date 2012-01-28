#ifndef QUERY_SYMBOLS_H__3705B467_12B3_421A_8F19_2001661E5EA6
#define QUERY_SYMBOLS_H__3705B467_12B3_421A_8F19_2001661E5EA6
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

#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/export.h"
#include "zdk/zero.h"
#include "zdk/debug_symbol_list.h"
#ifdef DEBUG
 #include "dharma/symbol_util.h"
#endif

namespace DebugSymbolHelpers
{
    class ZDK_LOCAL Events : public DebugSymbolEvents
    {
    public:
        BEGIN_INTERFACE_MAP(Events)
            INTERFACE_ENTRY(DebugSymbolEvents)
        END_INTERFACE_MAP()

        explicit Events(DebugSymbolList& list) : list_(list) {}

        virtual ~Events() {}

    private:
        DebugSymbolList& list_;

        bool notify(DebugSymbol* sym)
        {
            assert(sym && sym->name());

            list_.push_back(sym);

            return false; /* don't read */
        }

        int numeric_base(const DebugSymbol*) const { return 0; }

        bool is_expanding(DebugSymbol*) const { return false; }

        void symbol_change(DebugSymbol*, DebugSymbol*) { };
    };
}


/**
 * Query symbols in the scope of the current stack frame
 */
size_t inline ZDK_LOCAL query_debug_symbols
(
    RefPtr<Thread>      thread,
    const char*         name,
    RefPtr<Symbol>      func,   // function of scope
    DebugSymbolList&    output,
    LookupScope         scope
)
{
    size_t result = 0;

    CHKPTR(thread);
    CHKPTR(thread->stack_trace());

    if (Frame* stackFrame = thread->stack_trace()->selection())
    {
        if (Debugger* debugger = thread->debugger())
        {
            DebugSymbolHelpers::Events events(output);

            RefPtr<Symbol> current = stackFrame->function();

            if (!current || (func && current->value() != func->value()))
            {
                /*
                 * The func is not in the current scope, examine
                 * global variables only.
                 */
                result = debugger->enum_globals(thread.get(),
                                                name,
                                                current.get(),
                                                &events,
                                                scope);
            }
            else
            {
                /*
                 * Function is in scope, examine both globals and locals
                 */
                result = debugger->enum_variables(
                                                thread.get(),
                                                name,
                                                func.get(),
                                                &events,
                                                scope);
            }
        }
    }
    return result;
}

#endif // QUERY_SYMBOLS_H__3705B467_12B3_421A_8F19_2001661E5EA6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
