#ifndef SYMBOL_H__64CC73C9_BC1E_48CD_8D35_FDB97E814270
#define SYMBOL_H__64CC73C9_BC1E_48CD_8D35_FDB97E814270
//
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
#include "zdk/breakpoint.h"
#include "zdk/shared_string.h"


struct SymbolTable;



/**
 * Interface to a symbol in an executable or shared object
 * symbols table.
 * @note this class models the symbols used by the linker
 * and dynamic loader; for debug information symbols,
 * @see DebugSymbol
 */
DECLARE_ZDK_INTERFACE_(Symbol, ZObject)
{
    DECLARE_UUID("164de687-93be-45b3-91d5-2ddda72fc7c0")
    /**
     * @return the symbol table containing this symbol
     * @note this method is not thread safe
     */
    virtual SymbolTable* table(ZObjectManager*) const = 0;

    /**
     * @return the address in the symbol table; the address
     * in memory may differ by an offset (the address where
     * the module is loaded into memory).
     * @see Symbol::offset()
     */
    virtual Platform::addr_t value() const = 0;

    /**
     * @return symbol's actual address in virtual memory --
     * the symbol may be mapped at an address in the program
     * virtual memory space, other than the address in the
     * symbol table.
     * @see Symbol::offset()
     */
    virtual Platform::addr_t addr() const = 0;

    /**
     * @return offset from the symbol's table address
     * @note sometimes we need to construct a Symbol object
     * for a code address that is off from the nearest function.
     * The value() methods returns the value of the nearest
     * (lower bound) entry in the ELF symbol table.
     * The addr() method calculates the effective memory
     * address by adding together the value, the offset and
     * the SymbolTable::base()
     */
    virtual Platform::addr_t offset() const = 0;

    /**
     * @return the (possibly mangled) symbol name
     * @note never returns NULL
     */
    virtual SharedString* name() const = 0;

    /**
     * @return the demangled function name (for C++),
     * optionally including the formal parameters.
     * @note never returns NULL -- if the name could not be
     * demangled, then it returns the name()
     */
    virtual SharedString* demangled_name(bool params = true) const = 0;

    /**
     * Same as demangled_name, only it will return NULL when the
     * name cannot be demangled.
     */
    virtual SharedString* demangle(bool params = true) const = 0;

    /**
     * @return the source file name (if known, or the
     * name of the binary that contains it otherwise)
     * @note never returns NULL
     */
    virtual SharedString* file() const = 0;

    /**
     * @return the line number in source file
     */
    virtual uint32_t line() const = 0;

    /**
     * Distinguish between functions and variables or labels
     */
    virtual bool is_function() const = 0;

    /**
     * This method is for symbols that are defined inside dynamic
     * libraries that have not been loaded into memory yet. Mark
     * that a breakpoint has to be inserted at this symbol as soon
     * as the dynamic library is loaded.
     * @param runnable the thread for which to set
     * the breakpoint, or NULL if it applies to all threads.
     * @throw logic_error is the symbol is loaded in memory.
     */
    virtual void set_deferred_breakpoint(
        BreakPoint::Type,
        Runnable* = NULL,
        BreakPoint::Action* = NULL,
        Symbol* reserved = NULL) = 0;

    /**
     * For internal use
     */
    virtual void trim_memory() = 0;
};

// Copyright (c) 2004, 2006, 2007 Cristian L. Vlasceanu

#endif // SYMBOL_H__64CC73C9_BC1E_48CD_8D35_FDB97E814270
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
