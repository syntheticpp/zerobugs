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
#include <boost/python.hpp>
#include "zero_python/python_embed.h"
#include "zdk/get_pointer.h"
#include "zdk/symbol_map.h"
#include "collector.h"
#include "locked.h"
#include "marshaller.h"
#include "symbol_map.h"
#include "utility.h"

using namespace std;
using namespace boost;
using namespace boost::python;


typedef Collector<Symbol> SymbolCallback;


static void
lookup_(RefPtr<SymbolMap> symbols, string name, SymbolCallback* callback)
{
    // MainThreadScope scope;
    symbols->enum_symbols(name.c_str(), callback, SymbolTable::LKUP_DYNAMIC);
}


static list lookup_by_name(SymbolMap* symbols, const char* name)
{
    SymbolCallback callback;
#if 1
    ThreadMarshaller::instance().exec_command(
        bind(lookup_, symbols, name, &callback), __func__);
#else
    symbols->enum_symbols_by_name(name, &callback, true);
#endif
    return callback.get();
}


static void
lookup_by_addr_(RefPtr<SymbolMap> symbols, addr_t addr, RefPtr<Symbol>& result)
{
    RefPtr<Symbol> sym = symbols->lookup_symbol(addr);
    result.swap(sym);
}



static RefPtr<Symbol> lookup_by_addr(SymbolMap* symbols, addr_t addr)
{
    RefPtr<Symbol> sym;

    if (addr)
    {
        ThreadMarshaller::instance().exec_command(
            bind(lookup_by_addr_, symbols, addr, boost::ref(sym)), __func__);
    }
    return sym;
}


void export_symbol_map()
{
    class_<SymbolMap, bases<>, boost::noncopyable>("SymbolMap", no_init)
        .def("lookup", lookup_by_addr, "lookup symbol by address")
        .def("lookup", lookup_by_name,
            "lookup symbols by name, return a list of matches"
            )
        ;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
