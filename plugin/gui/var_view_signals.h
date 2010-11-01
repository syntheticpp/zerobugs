#ifndef VAR_VIEW_SIGNALS_H__886B4B9F_D670_4BFB_9F0B_85722F643A63
#define VAR_VIEW_SIGNALS_H__886B4B9F_D670_4BFB_9F0B_85722F643A63
//
// $Id: var_view_signals.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/export.h"
#include "gtkmm/box.h"


struct ZDK_LOCAL VariablesViewSignals : public Gtk::VBox
{
    // This signal is emitted for composite symbols
    // (class, array, etc) when they are expanded.
    SigC::Signal1<void, DebugSymbol*> symbol_expand;

    // emitted by the right-click menu
    SigC::Signal1<void, addr_t> show_raw_memory;

    SigC::Signal1<void, RefPtr<DebugSymbol> > set_watchpoint;

    SigC::Signal2<void, RefPtr<DebugSymbol>, int> describe_type;

    SigC::Signal2<bool, RefPtr<DebugSymbol>, const std::string&> edit;

    SigC::Signal0<void> numeric_base_changed;

    SigC::Signal2<void, RefPtr<DebugSymbol>, DebugSymbolEvents*> read_symbol;

    typedef SigC::Signal3<bool,
                          RefPtr<DebugSymbol>*,
                          RefPtr<DebugSymbol>,
                          DebugSymbolEvents*> DataFilterSignal;

    DataFilterSignal filter;
};


#endif // VAR_VIEW_SIGNALS_H__886B4B9F_D670_4BFB_9F0B_85722F643A63
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
