#ifndef SYMBOL_TABLE_BASE_H__58D399C6_8B8D_43AE_BD79_8BEE2DE0CBC7
#define SYMBOL_TABLE_BASE_H__58D399C6_8B8D_43AE_BD79_8BEE2DE0CBC7
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id$
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "dharma/config.h"
#include "dharma/hash_map.h"
#include "zdk/symbol_table.h"
#include "symbolz/public/symbol_table_events.h"



CLASS SymbolTableBase : public ZObjectImpl<SymbolTable>
{
public:
    ~SymbolTableBase() throw() { }

    DECLARE_UUID("ec318b59-341f-422f-85dc-b3987afe7605")

BEGIN_INTERFACE_MAP(SymbolTableBase)
    INTERFACE_ENTRY(SymbolTableBase)
    INTERFACE_ENTRY_AGGREGATE(events_)
    INTERFACE_ENTRY_INHERIT(ZObjectImpl<SymbolTable>)
END_INTERFACE_MAP()

    addr_t addr() const { return addr_; }
    addr_t upper() const { return upper_; }
    loff_t adjustment() const;

    Process* process(ZObjectManager*) const;
    Module* module();

    RefPtr<SymbolTableEvents> events() const { return events_; }

    bool set_deferred_breakpoint(Symbol&,
                                 BreakPoint::Type,
                                 Runnable*,
                                 BreakPoint::Action*);

    size_t enum_deferred_breakpoints(DeferredCallback*) const;

    void set_addr(addr_t, addr_t);
    void set_upper(addr_t upper) { upper_ = upper; }

protected:
    typedef google::dense_hash_map<addr_t, RefPtr<Symbol> > SymHash;
    typedef std::multimap<addr_t, WeakPtr<BreakPoint> > DeferredMap;

    SymbolTableBase(SymbolTableEvents&, Process*, addr_t addr, addr_t upper);

    RefPtr<SymbolTableEvents>   events_;
    mutable Mutex               mutex_;
    WeakPtr<Process>            process_;
    mutable addr_t              addr_;      // address where effectively loaded
    mutable addr_t              upper_;
    mutable loff_t              adjust_;
    DeferredMap                 deferredBreakpoints_;
    mutable SymHash             symHash_;   // cache lookup results
    mutable RefPtr<Module>      module_;
    mutable RefPtr<SymbolTable> next_;      // next table in file, if any
};
#endif // SYMBOL_TABLE_BASE_H__58D399C6_8B8D_43AE_BD79_8BEE2DE0CBC7
