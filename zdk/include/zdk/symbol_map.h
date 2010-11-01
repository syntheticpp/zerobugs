#ifndef SYMBOL_MAP_H__D71D455B_FEB3_49EB_A4F3_21D11DDFCA50
#define SYMBOL_MAP_H__D71D455B_FEB3_49EB_A4F3_21D11DDFCA50
//
// $Id: symbol_map.h 714 2010-10-17 10:03:52Z root $
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/ref_ptr_enforcer.h"
#include "zdk/symbol_table.h"


/**
 * Describes a dynamic library mapped in the debuggee's
 * memory space by the dynamic loader.
 */
DECLARE_ZDK_INTERFACE_(SymbolMapLinkData, RefCounted)
{
    virtual SharedString* filename() const = 0;

    virtual Platform::addr_t addr() const = 0;

    virtual off_t adjustment() const = 0;

    virtual SymbolMapLinkData* next() const = 0;

    virtual SymbolTable* table() const = 0;
};


/**
 * A collection of symbol tables, that correspond to modules
 * mapped in the virtual memory at various addresses.
 * @see Module
 */
DECLARE_ZDK_INTERFACE_(SymbolMap, ZObject)
{
    DECLARE_UUID("2fdbce4b-1f52-4950-ab7e-f54604b85828")

	typedef SymbolMapLinkData LinkData;

    /**
     * Update the symbol map's content.
     * The caller passes in a linked list of LinkData (for example,
     * the list may be obtained by reading the r_debug struct).
     */
    virtual void update(const LinkData*) = 0;

    /**
     * Retrieve a list of mapped files.
     * @see LinkData
     */
    virtual LinkData* file_list(ENFORCE_REF_PTR_) const = 0;

    /**
     * Get a linked list of symbols tables by their binary filename;
     * the filename can correspond to the program itself or to
     * a shared object, and has to match exactly one of the names
     * that was passed into the read method.
     * @return the head of the list, or NULL if none found.
     */
    virtual SymbolTable* symbol_table_list(const char* fname) const = 0;

    /* @note: called once per list head */
    virtual size_t enum_symbol_tables(EnumCallback<SymbolTable*>*) const = 0;

    /**
     * Lookup symbols, by name, inside modules (shared libs) that
     * are not loaded into the debugged programs memory yet, but
     * appear in the RT_NEEDED section.
     */
    virtual size_t enum_needed_symbols( const char* name,
                                        EnumCallback<Symbol*>*,
                                        SymbolTable::LookupMode) = 0;
    /**
     * Enumerate symbol tables that correspond to shared objects
     * that have not been loaded yet.
     */
    virtual size_t enum_needed_tables(EnumCallback<SymbolTable*>*) const = 0;

    /**
     * Explicitly adds a module (shared library, executable) to
     * the internal list of needed libraries. To understand the
     * need for this method, imagine the following use case: the
     * user wants to set a breakpoint at function "Fun".
     * The debugger engine calls the enum_symbols method,
     * and sets a breakpoint at all matching functions.
     * Now, enum_symbols only works for the
     * modules (again, think "shared libraries") that are currently
     * loaded. If no match is found, the engine may call
     * enum_needed_symbols(), which will go through the RT_NEEDED
     * section, and look at the symbol tables of shared libraries that
     * are needed, but have not been loaded into memory just yet.
     * However, if a program calls dlopen() explicitly (rather than
     * implicitly linking with a shared library), then the said
     * library will not be listed in the RT_NEEDED section. The
     * solution is to present the user of the debugger engine with
     * this method, which explicitly adds the module (aka shared
     * library) to the list of needed libraries.
     * @return true if successfully added,
     * false otherwise (maybe because already added).
     */
    virtual bool add_module(const char*) = 0;

    /**
     * Looks up a symbol by address, returns it if found, of NULL
     */
    virtual Symbol* lookup_symbol(Platform::addr_t) const = 0;

    /**
     * Lookup the table that contains the given address, and return it
     * if found, or return NULL otherwise.
     */
    virtual SymbolTable* lookup_table(Platform::addr_t) const = 0;

    /**
     * Enumerates over all symbol tables, the EnumCallback
     * is called for every symbol that matches the passed name.
     * @see SymbolTable::enum_symbols()
     * @param pattern is interpreted according to the lookup mode
     * @return the number of matching symbols.
     */
    virtual size_t enum_symbols(
        const char* pattern,
        EnumCallback<Symbol*>* = 0,
        SymbolTable::LookupMode = SymbolTable::LKUP_SYMBOL) = 0;
};



#ifdef DEBUG
/*  Debug macros -- for enforcing that naked
    pointers to RefCounted objects, returned from functions,
    are being wrapped into RefPtr<> by the callers. */

 #define file_list() file_list(&RefPtrEnforcer(__FILE__, __LINE__))

#else

 #define file_list() file_list(0)
#endif


#endif // SYMBOL_MAP_H__D71D455B_FEB3_49EB_A4F3_21D11DDFCA50
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
