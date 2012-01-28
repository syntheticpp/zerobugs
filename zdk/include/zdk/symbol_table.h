#ifndef SYMBOL_TABLE_H__FE395759_310E_4DC4_B4A1_13B572B9BD8F
#define SYMBOL_TABLE_H__FE395759_310E_4DC4_B4A1_13B572B9BD8F
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

#include "zdk/enum.h"
#include "zdk/symbol.h"

struct Module;
struct Process;


typedef EnumCallback2<BreakPoint*, const SymbolTable*> DeferredCallback;

/**
 * Interface for reading the symbol tables from
 * binary modules as programs and shared libs.
 * @see Module, Symbol, SymbolMap
 */
DECLARE_ZDK_INTERFACE_(SymbolTable, ZObject)
{
    DECLARE_UUID("55293cb9-9f19-4467-a6a5-7402c357d7bf")

    enum LookupMode
    {
        LKUP_SYMBOL =       0x0000,
        LKUP_DYNAMIC =      0x0001,
        LKUP_UNMAPPED =     0x0002,
        LKUP_ISMANGLED =    0x0004, // the pattern is a mangled symbol name
        LKUP_RANGE =        0x0008,
        LKUP_REGEX =        0x0010,
        LKUP_DEMANGLED =    0x0020,
    };

    virtual SharedString* name() const = 0;

    /**
     * @return the filename of the program or shared library
     * that contains this table.
     *
     * @param followLink if true, return the .gnu_debuglink name,
     * 	if any, otherwise return program or shared lib file name
     */
    virtual SharedString* filename(bool followLink = false) const = 0;

    /**
     * @return true if symbols in this table are loaded  BY THE DEBUGGER
     *
     * @note the name is a bit ambiguous, not to be mistaken for
     * "loaded by the debuggee"
     */
    virtual bool is_loaded() const = 0;

    /**
     * Delta between the address where the module is loaded
     * and the load address specified by the shared object.
     */
    virtual int64_t adjustment() const = 0;

    /**
     * Address where effectively loaded
     */
    virtual Platform::addr_t addr() const = 0;

    virtual Platform::addr_t upper() const = 0;

    virtual bool is_dynamic() const = 0;

    virtual bool is_virtual_shared_object() const = 0;

    /**
     * @return the number of symbols in the table.
     */
    virtual size_t size() const = 0;

    /**
     * Lookup symbol by address, return NULL if not found.
     * @note The caller must wrap the pointer into a RefPtr.
     * @note This method expects a virtual memory address,
     *  i.e. it subtracts adjustment() from the value passed in.
     */
    virtual Symbol* lookup_symbol(Platform::addr_t) const = 0;

    /**
     * Enumerate the symbols defined in the given source
     * file, at the given line; the EnumCallback is notified
     * for all symbols that match.
     *
     * @return count of symbols
     */
    virtual size_t enum_addresses_by_line(
            SharedString*   file,
            size_t          line,
            EnumCallback<Platform::addr_t>* observer) const = 0;

    /**
     * Enumerate all symbols that match the specified name.
     * The EnumCallback::notify method is repetitively called
     * for all symbols that match the passed name. A custom
     * implementation of the interface could, for example,
     * store the matching symbols.
     * @param name enumerate only those symbols that match
     * the name. If NULL is passed in for the name, then all
     * symbols are enumerated.
     * @param observer function object, implemented by client
     * code, gets notified for every enumerated symbol.
     * @return the count of matching symbols.
     */
    virtual size_t enum_symbols(
            const char* name,
            EnumCallback<Symbol*>* observer = 0,
            LookupMode = LKUP_SYMBOL) const = 0;

    /**
     * @return the next symbol table in the file
     */
    virtual SymbolTable* next() const = 0;

    /**
     * For symbol tables corresponding to dynamic libraries
     * that have not yet been loaded into memory -- enumerate
     * the symbols where a breakpoint needs to be inserted at
     * as soon as the dynamic library gets loaded.
     * @throw logic_error if the table is mapped (i.e. addr() != 0)
     */
    virtual size_t enum_deferred_breakpoints(DeferredCallback* = 0) const = 0;

    /**
     * @return the process this table belongs to
     */
    virtual Process* process(ZObjectManager*) const = 0;

    virtual Module* module() = 0;

    virtual void ensure_loaded() const = 0;
};


inline ZDK_LOCAL SymbolTable::LookupMode
operator | (SymbolTable::LookupMode lhs, SymbolTable::LookupMode rhs)
{
    return static_cast<SymbolTable::LookupMode>(lhs | static_cast<int>(rhs));
}


#endif // SYMBOL_TABLE_H__FE395759_310E_4DC4_B4A1_13B572B9BD8F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
