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
#include <string>
#include <boost/python.hpp>
#include "zdk/get_pointer.h"
#include "zdk/process.h"
#include "zdk/symbol_table.h"
#include "collector.h"
#include "locked.h"
#include "marshaller.h"
#include "module.h"
#include "symbol_table.h"


using namespace std;
using namespace boost;
using namespace boost::python;


typedef Collector<Symbol> SymbolCallback;


static string symbol_table_filename(const SymbolTable* table)
{
    string filename;
    if (SharedString* fname = table->filename())
    {
        filename.assign(fname->c_str());
    }
    return filename;
}


/**
 * Lookup symbol by address on main debugger thread.
 */
static void
lookup_by_addr_(RefPtr<SymbolTable> symtab, addr_t addr, RefPtr<Symbol>& result)
{
    RefPtr<Symbol> sym = symtab->lookup_symbol(addr);
    result.swap(sym);
}



static RefPtr<Symbol> lookup_by_addr(SymbolTable* symtab, addr_t addr)
{
    RefPtr<Symbol> sym;

    if (addr)
    {
        ThreadMarshaller::instance().send_command(
            bind(lookup_by_addr_, symtab, addr, boost::ref(sym)),
            __func__);
    }
    return sym;
}


static void
lookup_by_name_(SymbolTable* symtab, string name, SymbolCallback* callback)
{
    // MainThreadScope scope;
    symtab->enum_symbols(name.c_str(), callback, SymbolTable::LKUP_DYNAMIC);
}


static list lookup_by_name(SymbolTable* symbols, const char* name)
{
    SymbolCallback callback;

    ThreadMarshaller::instance().exec_command(
        bind(lookup_by_name_, symbols, name, &callback),
        __func__);

    return callback.get();
}


class NullManager : public ZObjectManager
{
    void manage(ZObject*) { }

    bool query_interface(uuidref_t, void**) { return false; }
};

// hmm, this may leak
static Process* get_process(const SymbolTable* table)
{
    NullManager phb;
    return table->process(&phb);
}


void export_symbol_table()
{
    class_<SymbolTable, bases<>, noncopyable>("SymbolTable", no_init)
        .def("__len__", &SymbolTable::size, "", locked<>())
        .def("addr", &SymbolTable::addr, "", locked<>())
        .def("filename", symbol_table_filename)
        .def("is_dynamic", &SymbolTable::is_dynamic)
        .def("lookup", lookup_by_addr, "lookup symbol by address")
        .def("lookup", lookup_by_name, "lookup symbol by name")
        .def("process", get_process,
            "get the process to which this table belongs",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("module", &SymbolTable::module,
            "return the module that owns this table",
            locked<return_value_policy<reference_existing_object> >()
            )
        .def("next", &SymbolTable::next,
            "get next table",
            locked<return_value_policy<reference_existing_object> >()
            )
        ;

    register_ptr_to_python<RefPtr<SymbolTable> >();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
