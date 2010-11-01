#ifndef SYMBOL_TABLE_EVENTS_H__7528196F_FF15_451A_8047_C6C5DD721B7D
#define SYMBOL_TABLE_EVENTS_H__7528196F_FF15_451A_8047_C6C5DD721B7D
//
// $Id: symbol_table_events.h 714 2010-10-17 10:03:52Z root $
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
#include "zdk/module.h"
#include "zdk/symbol_table.h"
#include "zdk/shared_string.h"
#include "zdk/zobject_impl.h"

using Platform::addr_t;


CLASS SymbolTableEvents : public ZObject, public DeferredCallback
{
protected:
    SymbolTableEvents() { }

public:
    DECLARE_UUID("54df6183-b465-4cf2-9051-17beaf2662fb")

    static RefPtr<SymbolTableEvents> create()
    { return new ZObjectImpl<SymbolTableEvents>(); }

    virtual ~SymbolTableEvents() throw() { }

    virtual bool use_lazy_loading() const;

    virtual void on_init(SymbolTable&) { }
    virtual void on_done(SymbolTable&) { }

    /**
     * This method is called by the SymbolTable implementation
     * as it is reading symbols. If the Events implementation
     * returns true here, the symbol is kept in the table, otherwise
     * it is discarded.
     */
    virtual bool on_symbol(SymbolTable&, Symbol&);

    /**
     * Lookup the source file and line where the symbol of
     * the given address is defined.
     *
     * @return line number if found, and set fileName
     * @param table the calling symbol table
     * @param addr address for which the source line
     *  needs to be returned
     * @param fileName upon return, will contain the name
     * of the source file that has generated the code at
     * the given address.
     */
    virtual size_t addr_to_line(
                const SymbolTable&      table,
                Platform::addr_t        addr,
                RefPtr<SharedString>&   fileName);

    virtual size_t line_to_addr(
                const SymbolTable&      table,
                RefPtr<SharedString>    sourceFileName,
                size_t                  sourceLine,
                EnumCallback<addr_t>*   observer);

    virtual addr_t next_line(const RefPtr<Symbol>&) const
    { return 0; }

    virtual RefPtr<Module> get_module(SymbolTable&) const
    {
        return RefPtr<Module>();
    }
    //
    // for deferred breakpoints
    //
    virtual void notify(BreakPoint*, const SymbolTable*)
    { }

    virtual BreakPoint* set_deferred_breakpoint(
        Runnable*,
        BreakPoint::Type,
        Symbol&,
        BreakPoint::Action*)
    {
        return NULL;
    }

    /**
     * useful when sources have moved from the original build location
     */
    virtual bool map_path(Process*, std::string& path) const
    {
        return false;
    }

    /**
     * @note experimental, may go away in the future
     */
    /*
    virtual size_t enum_tables_by_source(
        SharedString* source,
        EnumCallback<SymbolTable*>*) const = 0;
    */
};


#endif // SYMBOL_TABLE_EVENTS_H__7528196F_FF15_451A_8047_C6C5DD721B7D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
