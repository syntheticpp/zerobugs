#ifndef CODE_VIEW_SIGNALS_H__F8902B08_2E4A_4BE8_9834_5A72C042C110
#define CODE_VIEW_SIGNALS_H__F8902B08_2E4A_4BE8_9834_5A72C042C110
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
//
#include "zdk/debug_symbol_list.h"
#include "zdk/ref_ptr.h"
#include "gtkmm/signal.h"


class DataType;
class DebugSymbolEvents;
class Frame;
class InterThreadCommand;
class RightClickInfo;
class MemoryRequest;
class Thread;


struct CodeViewSignals
{
    SigC::Signal2<void, RightClickInfo*, bool> set_breakpoints;
    SigC::Signal1<void, addr_t> set_program_count;

    SigC::Signal2<void, addr_t, size_t> delete_breakpoint;
    SigC::Signal2<void, addr_t, size_t> disable_breakpoint;
    SigC::Signal2<void, addr_t, size_t> enable_breakpoint;

    SigC::Signal2<void, addr_t, bool> show_next_line;

    //query symbols by name, if last parameter is true, read their values
    SigC::Signal4<void, std::string, addr_t, DebugSymbolList*, bool> query_symbols;

    SigC::Signal0<bool> can_interact;

    SigC::Signal1<void, DebugSymbolList> evaluate;

    // emitted by search functions
    SigC::Signal1<bool, const std::string&> string_not_found;

    // emitted when about to disassemble
    SigC::Signal1<void, RefPtr<MemoryRequest> > read_memory_async;
    SigC::Signal2<void, RefPtr<DebugSymbol>, DebugSymbolEvents*> read_symbol;

    // emitted when the current symbol in view changes
    SigC::Signal3<void, RefPtr<Thread>, RefPtr<Symbol>, Frame*> symbol_changed;

    SigC::Signal1<void, std::string> status_message;

    SigC::Signal1<bool, RefPtr<InterThreadCommand> > run_on_main_thread;

    SigC::Signal1<void, RefPtr<Symbol> > step_over_func;
    SigC::Signal1<void, RefPtr<Symbol> > step_over_file;
    SigC::Signal1<void, RefPtr<Symbol> > step_over_dir;
    SigC::Signal1<void, RefPtr<Symbol> > step_over_reset;
    SigC::Signal1<void, RefPtr<Symbol> > step_over_manage;

    SigC::Signal2<bool, const char*, size_t> accept_invalid_utf8;

    SigC::Signal1<void, RefPtr<DataType> > what_is;

    typedef SigC::Signal3<bool,
                          RefPtr<DebugSymbol>*,
                          RefPtr<DebugSymbol>,
                          DebugSymbolEvents*> DataFilterSignal;

    DataFilterSignal filter;
    virtual void on_read_done(RefPtr<MemoryRequest>) = 0;
};
#endif // CODE_VIEW_SIGNALS_H__F8902B08_2E4A_4BE8_9834_5A72C042C110
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
