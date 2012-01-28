#ifndef SYMBOL_EVENTS_H__3D013F3F_4A6C_45DC_8DBA_FE4281C241BB
#define SYMBOL_EVENTS_H__3D013F3F_4A6C_45DC_8DBA_FE4281C241BB
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
#include <vector>
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "zdk/mutex.h"
#include "symbolz/public/symbol_table_events.h"


class DebuggerEngine;
class DebuggerPlugin;
class DebugInfoReader;


/*
 * Helper callbacks invoked by the symbol tables via the
 * SymbolTableEvents interface.
 */
CLASS SymbolEvents : public ZObjectImpl<SymbolTableEvents>
                   , EnumCallback<DebuggerPlugin*>
                   , EnumCallback<TranslationUnit*, bool>
                   , EnumCallback<SharedString*, bool>

{
    // cache the debug info readers
    typedef std::vector<DebugInfoReader*> DebugInfo;

    // map source filenames to the symbol tables of
    // their executable or shared object(s)
    // ***** NOTE: experimental feature, not used yet *****
    typedef ext::hash_multimap<RefPtr<SharedString>, WeakPtr<SymbolTable> > SourceMap;

   typedef ext::hash_map<RefPtr<SharedString>, RefPtr<SharedString> > PathMap;

public:
    explicit SymbolEvents(DebuggerEngine& engine);
    virtual ~SymbolEvents() throw();

BEGIN_INTERFACE_MAP(SymbolEvents)
    INTERFACE_ENTRY(SymbolTableEvents)
    INTERFACE_ENTRY_AGGREGATE(engine_.properties())
END_INTERFACE_MAP()

protected:
    bool use_lazy_loading() const;

    /**
     * notification that a table is about to be read
     */
    void on_init(SymbolTable&);

    /**
     * notification that a table has been read
     */
    void on_done(SymbolTable&);

    bool on_symbol(SymbolTable&, Symbol&);

    size_t addr_to_line(
                const SymbolTable&      table,
                addr_t                  addr,
                RefPtr<SharedString>&   fileName);

    size_t line_to_addr(
                const SymbolTable&      table,
                RefPtr<SharedString>    sourceFile,
                size_t                  sourceLine,
                EnumCallback<addr_t>*   observer);

    addr_t next_line(const RefPtr<Symbol>&) const;

    DebuggerEngine& engine() const { return engine_; }

    void notify(DebuggerPlugin*);

    RefPtr<Module> get_module(SymbolTable&) const;

    void notify(BreakPoint*, const SymbolTable*);

    BreakPoint* set_deferred_breakpoint(
        Runnable*,
        BreakPoint::Type,
        Symbol&,
        BreakPoint::Action*);

    //size_t enum_tables_by_source(
    //    SharedString*,
    //    EnumCallback<SymbolTable*>*) const;

    void map_sources(SymbolTable&);

    bool notify(TranslationUnit*);
    bool notify(SharedString*);

    bool map_path(Process*, std::string& path) const;
    SharedString* reverse_map_path(const RefPtr<SharedString>&) const;

private:
    DebuggerEngine& engine_;
    bool            loadDynamicSyms_; // keep dynamic syms?
    DebugInfo       debugInfo_;
    addr_t          prevAddr_;
    mutable Mutex   mutex_;
    SourceMap       sourceMap_;
    mutable PathMap pathMap_;
    WeakPtr<SymbolTable> currentTable_;
};


#endif // SYMBOL_EVENTS_H__3D013F3F_4A6C_45DC_8DBA_FE4281C241BB
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
