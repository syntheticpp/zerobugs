#ifndef SYMBOL_MAP_IMPL_H__138BC994_43A2_4A8A_99B6_36108BB1C36B
#define SYMBOL_MAP_IMPL_H__138BC994_43A2_4A8A_99B6_36108BB1C36B
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
#include <map>
#include <string>
#include <vector>
#include "zdk/mutex.h"
#include "zdk/symbol_map.h"
#include "zdk/zobject_impl.h"
#include "symbolz/private/symbol_table_impl.h"
#include "symbolz/public/symbol_map.h"
#include "symbolz/public/symbol_table_events.h"

#undef file_list


/**
 * Implement map of symbol tables. Each module mapped in the process space
 * has one or more symbol tables.
 */
CLASS SymbolMapImpl : public ZObjectImpl<SymbolMap>
{
public:
    // index symbol table by the starting address where the
    // files containing them are mapped into the virtual memory
    typedef std::map<addr_t, RefPtr<SymbolTable> > SymbolTableMap;

    typedef std::map<std::string, RefPtr<SymbolTableBase> > SymbolTableGroup;

    /**
     * Constructs the symbol map for a core file.
     */
    SymbolMapImpl(Process&, const ELF::CoreFile&, SymbolTableEvents&);

    /**
     * Constructs the symbol map of a running program.
     */
    SymbolMapImpl(Process&, SymbolTableEvents&);

    virtual ~SymbolMapImpl() throw();

    void read();

    Process* process() const { return process_.get(); }

protected:
    void update(const LinkData*);

    LinkData* file_list(ENFORCE_REF_PTR_) const;

    RefPtr<SymbolTable> read_tables(addr_t, size_t, const std::string&);

    RefPtr<SymbolTable> read_tables(addr_t,
                                    size_t,
                                    const std::string&,
                                    SymbolTableMap&);
    void ensure_tables_loaded();

    /**
     * @return the symbol table that corresponds to the passed
     * file name, or NULL if no such table read.
     */
    SymbolTable* symbol_table_list(const char* fname) const;

    size_t enum_symbol_tables(EnumCallback<SymbolTable*>*) const;

    size_t enum_needed_tables(EnumCallback<SymbolTable*>*) const;

    bool add_module_internal(const char*) const;

    bool add_module(const char* filename)
    {
        return add_module_internal(filename);
    }

    /**
     * Looks up a symbol by address, returns it if found, of NULL
     */
    Symbol* lookup_symbol(addr_t) const;

    SymbolTable* lookup_table(addr_t) const;

    size_t enum_symbols(
        const char* filename,
        EnumCallback<Symbol*>* callback,
        SymbolTable::LookupMode);

    size_t enum_needed_symbols(
        const char* filename,
        EnumCallback<Symbol*>* callback,
        SymbolTable::LookupMode);

    addr_t get_load_addr() const;

    void scan_needed_tables() const;

    void scan_needed_tables(const std::string&) const;

    /**
     * map shared object at address
     */
    bool map_library(SymbolTableMap&,
                     const std::string&,
                     addr_t effectiveAddr,
                     addr_t elfPreferredLoadAddr,
                     size_t memMappedSize);

    /**
     * Canonicalize filename and search the tables
     */
    bool is_mapped(const char* filename) const;
 
    bool is_needed(const std::string& filename) const;

private:
    /**
     * Look at the specified module's ELF info and add the tables
     * that correspond to needed libraries.
     */
    void add_needed_tables(const char* moduleFileName) const;

    /**
     * This is a linux-specific hack: a virtual Dynamic Shared Object
     * (exported by the kernel) is mapped in the process' address
     * space (with newer systems). If a symbol lookup in the "normal"
     * tables fails, fallback to this method.
     */
    RefPtr<Symbol> vdso_lookup(addr_t) const;

private:
    typedef std::auto_ptr<DynLibList> DynLibListPtr;

    SymbolTableEvents&          events_;        // observer
    RefPtr<Process>             process_;       // TODO make it a WeakPtr

    // mutable data is updated in enum_symbol_tables, enum_needed_tables
    mutable long                arch_;
    mutable SymbolTableMap      map_;
    mutable addr_t              loadAddr_;      // addr where program is loaded
    mutable Mutex               mutex_;

    // caches result of file_list: do not use for anything else
    mutable RefPtr<LinkData>    mappedFiles_;
    mutable SymbolTableGroup    neededTables_;
    mutable bool                scannedNeededTables_;

    mutable RefPtr<SymbolTable> vdsoTables_;
    mutable DynLibListPtr       dynLibs_;
};

#endif // SYMBOL_MAP_IMPL_H__138BC994_43A2_4A8A_99B6_36108BB1C36B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
