#ifndef DEBUG_SYM_H__A7F13D4A_8183_455B_9739_BEB0D033A09F
#define DEBUG_SYM_H__A7F13D4A_8183_455B_9739_BEB0D033A09F

// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

/* Interfaces for debug symbol info readers.
   A plug-in may implement the DebugInfoReader interface.

   The main idea is to abstract out the differences
   between various debug formats (stabs, dwarf, etc.)
   under a common set of C++ interfaces.

   Copyright (c) 2004, 2006, 2007 Cristian L. Vlasceanu
 */
#include "zdk/enum.h"
#include "zdk/misc.h"
#include "zdk/platform.h"
#include "zdk/ref_ptr_enforcer.h"
#include "zdk/translation_unit.h"
#include "zdk/zerofwd.h"

using Platform::addr_t;
using Platform::bitsize_t;



DECLARE_ZDK_INTERFACE_(DebugSymbolCallback, Unknown2)
{
    DECLARE_UUID("941ce6f7-a496-49f3-95f8-d7e7215bfd79")

    /**
     * Notified when a new symbol (possibly the child of an
     * aggregated object such a class instance or array) is
     * detected; if the method returns true, the symbol's
     * value is read.
     */
    virtual bool notify(DebugSymbol*) = 0;
};


/**
 * This interface gets notified/interrogated when
 * debug symbols are being read or enumerated.
 */
DECLARE_ZDK_INTERFACE_(DebugSymbolEvents, DebugSymbolCallback)
{
    DECLARE_UUID("711be8dd-856e-4ff0-a577-5a9e88bfdefc")
    /**
     * Notified when a new symbol (possibly the child of an
     * aggregated object such a class instance or array) is
     * detected; if the method returns true, the symbol's
     * value is read.
     */
    virtual bool notify(DebugSymbol*) = 0;

    /**
     * Symbols that correspond to aggregate objects such as
     * class instances or arrays may be expanded, so that the
     * user can inspect their sub-parts. This method is called
     * by the reader implementations to determine if the client
     * wants such an aggregate object to be expanded or not.
     */
    virtual bool is_expanding(DebugSymbol*) const = 0;

    /**
     * Readers call this method to determine what numeric base
     * should be used for the representation of integer values.
     */
    virtual int numeric_base(const DebugSymbol*) const = 0;

    /**
     * A change in the symbol has occurred (name, type, address
     * etc.) A pointer to the old values is passed in.
     */
    virtual void symbol_change
      (
        DebugSymbol* newSym,
        DebugSymbol* old
      ) = 0;
};


struct DebugInfoReader;


/**
 * Abstracts out a debug symbol, insulates the
 * application from the actual format (STABS, DWARF, etc.)
 * @note this is not the same as the Symbol interface, which
 * represents entries in the ELF symbol table. There are
 * two kinds of symbol tables that a binary module may have --
 * for the linker use, or for debugger use. The artifacts
 * that are for linker use are represented by the Symbol and
 * SymbolTable interfaces. This interface models symbols that
 * are for debugger's use.
 */
DECLARE_ZDK_INTERFACE_(DebugSymbol, ZObject)
{
    DECLARE_UUID("a8408abb-827b-4169-a7eb-498d88aede33")

    /**
     * The name of the variable represented by this symbol
     */
    virtual SharedString* name() const = 0;

    virtual DataType* type() const = 0;

    virtual addr_t addr() const = 0;

    /**
     * For aggregated symbols such as classes, structs etc.
     */
    virtual DebugSymbol* parent() const = 0;

    /**
     * Enumerate the member data of variables of aggregate types
     * (classes, unions, arrays, etc), calling the provided callback
     * (if not NULL) for every child.
     * @return the number of children
     */
    virtual size_t enum_children(DebugSymbolCallback* = 0) const = 0;

    /**
     * @return the n-th child
     * @throw out_of_range
     */
    virtual DebugSymbol* nth_child(size_t n) = 0;

    /**
     * The symbol's value, formatted as a string.
     */
    virtual SharedString* value() const = 0;

    /**
     * Indicates whether this debug symbol corresponds to a value
     * returned from a function (as opposed to a parameter or variable)
     */
    virtual bool is_return_value() const = 0;

    virtual bool is_constant() const = 0;

    virtual int compare(const DebugSymbol*) const = 0;

    /**
     * The thread context in which the variable is read.
     */
    virtual Thread* thread() const = 0;

    /**
     * @return the depth of this symbol -- applies to child
     * symbols of class or struct instances, pointed objects,
     * array elems.
     */
    virtual size_t depth() const = 0;

    virtual void add_child(DebugSymbol*) = 0;

    /**
     * Updates the current value of the symbol by reading it
     * from the debugged thread's memory.
     */
    virtual void read(DebugSymbolEvents*, long reserved = 0) = 0;

    /**
     * Parse the given string according to this symbol's type,
     * and convert it to a value of that type, then write the
     * value to the symbol's location in memory (or in a CPU
     * register, in some cases).
     * @return the number of parsed characters.
     * @note if the resulting number of parsed characters is
     * not equal to the length of the string, the input string
     * is treated as erroneous and the memory remains unchanged.
     */
    virtual size_t write(const char* str) = 0;

    /**
     * Create a replica of this debug symbol which inherits all
     * attributes of this symbol but the specifically overriden ones
     */
    virtual DebugSymbol* clone
      (
        SharedString* value = 0,
        DataType* type = 0,
        bool makeConst = true
      ) const = 0;

    /**
     * @return the reader object that produced this symbol
     */
    virtual DebugInfoReader* reader() const = 0;

    /**
     * @return line number in source file where declared, if
     * the information is available, otherwise return 0.
     */
    virtual size_t decl_line() const = 0;

    /**
     * @return source file name where declared, if
     * the information is available, otherwise return NULL.
     */
    virtual SharedString* decl_file() const = 0;

    /**
     * normally return type()->name(), but allows the impl
     * to override the datatype name
     */
    virtual SharedString* type_name() const = 0;

    virtual const char* tooltip() const = 0;
};


/**
 * Interface to debug info readers
 */
DECLARE_ZDK_INTERFACE_(DebugInfoReader, Unknown2)
{
    DECLARE_UUID("31372121-a5f9-4639-a4fe-d6a9df3edaa7")

    /**
     * Return a string describing the debug format
     * supported by this plugin (e.g. "dwarf").
     */
    virtual const char* format() const = 0;

    /**
     * Enumerates the data objects visible in the scope of the
     * given thread and function. If the EVENTS pointer is not
     * NULL, then its NOTIFY method is invoked for every enumerated
     * DebugSymbol.
     * @return the count of enumerated symbols.
     * @note if name is not NULL, then only variables strictly
     * matching the name are enumerated.
     */
    virtual size_t enum_locals
      (
        Thread* thread,
        const char* name,
        Frame* frame,
        Symbol* func,
        DebugSymbolEvents* events,
        bool paramOnly = false
      ) = 0;

    /**
     * Enumerate global variables
     * @param thread
     * @param name if not NULL, filter by exact matching name
     * @param func current function in scope
     * @param events receives notifications for each global
     * @param scope
     * @param enumFunctions
     * @return number of enumerated globals
     */
    virtual size_t enum_globals
      (
        Thread* thread,
        const char* name,
        Symbol* func,
        DebugSymbolEvents* events,
        LookupScope = LOOKUP_ALL,
        bool enumFunctions = false
      ) = 0;

    ////////////////////////////////////////////////////////////
    ///
    /// Methods for source/line retrieval
    ///
    /**
     * Lookup the source file and line number that match the
     * given address.
     * @param symTable the head of a linked list of symbol tables
     * (static and dynamic) for the current module.
     * @param addr the address to lookup
     * @param nearest if not NULL, it is filled out with the
     * closest address that has a corresponding line number entry
     * in the debug info (STABS, DWARF, or possible other formats)
     * and nearest lines are returned if an exact match is not found.
     * When this parameter is NULL, exact lookups are performed.
     * @param cb pointer to  callback functor, whose `notify' method
     * gets called repeatedly for all matches (if not NULL).
     * @return non-zero if there are any matches found for addr
     */
    virtual size_t addr_to_line
      (
        const SymbolTable*  symTable,
        addr_t              addr,
        addr_t*             nearest,
        EnumCallback2<SharedString*, size_t>* cb
      ) = 0;

    /**
     * @return the first address generated for the
     * source line following the line where given
     * symbol is defined (supports the debugger's
     * `next' command).
     */
    virtual addr_t next_line_addr(  const SymbolTable*,
                                    addr_t addr,
                                    SharedString* file,
                                    size_t line) const = 0;

    /**
     * Enumerate addresses generated for given source line.
     * @return the number of addresses that match.
     */
    virtual size_t line_to_addr
      (
        Process*                process,
        SharedString*           moduleName,
        loff_t                  moduleAdjustment,
        SharedString*           sourceFile,
        size_t                  lineNumber,
        EnumCallback<addr_t>*   observer,
        bool*                   cancelled = NULL
      ) = 0;

    /**
     * This is a helper function, intended to be called by
     * Thread::ret_addr(). Normally, when stepping thru a
     * function, the debugger finds the return address of the
     * function on the stack. This however may not work when
     * the compiler has optimized the code, by inlining a
     * function, for example. The method here is a hook for
     * such cases.
     * @note Implementations of this method must NEVER call
     * Thread::ret_addr(). If the method does not find any
     * return address information, it should return false; the
     * engine will then read the return address from the stack.
     * If return address information is found, the method
     * returns true and fills out the return address.
     * @param thread execution context
     * @param addr a code address inside a function
     * @param retAddr will contain the return address
     */
    virtual bool get_return_addr(Thread*  thread,
                                addr_t    addr,
                                addr_t*   retAddr) const = 0;

    /**
     * Given a SymbolTable entry, that corresponds to a
     * function construct a debug symbol that corresponds
     * to its return value.
     */
    virtual DebugSymbol* get_return_symbol
      (
        Thread*         thread,
        const Symbol*   symbol,
        ENFORCE_REF_PTR_
      ) = 0;

    /**
     * Lookup the function that contains the given address.
     * @param thread current thread of execution
     * @param addr address to look up
     * @param begin upon successful return, is filled out
     * with the address where the function begins
     * @param end upon successful return, is filled out
     * with the address where the function ends
     */
    virtual bool get_fun_range
      (
        Thread* thread,
        addr_t  addr,
        addr_t* begin,
        addr_t* end
      ) const = 0;

    /**
     * Lookup a type by name, in the context of a given thread.
     * @param thread
     * @param name
     * @param addr base address for address calculations (for DWARF)
     *  If no address is specified, the program counter of the
     *  current frame is used.
     */
    virtual DataType* lookup_type
      (
        Thread* thread,
        const char* name,
        addr_t addr = 0,
        LookupScope = LOOKUP_ALL
      ) = 0;

    /**
     * Lookup the compilation unit that has the given
     * address in its range of addresses [lower_addr, upper_addr)
     * @param filename module (executable or DSO) name
     * @param addr address inside compilation unit
     */
    virtual TranslationUnit* lookup_unit_by_addr
      (
        Process*,
        SharedString* moduleFileName,
        addr_t addr
      ) = 0;

    /**
     * Lookup compilation units by file name (which is expected
     * to be in a canonical form).
     * If the given source file is included by several compilation
     * units, they are all passed to the EnumCallback's notify
     * method.
     * @note the enumeration of units is cancelled if notify
     * returns false
     * @return the count of enumerated units.
     */
    virtual size_t lookup_unit_by_name
      (
        Process*,
        SharedString* moduleFileName,
        const char* unitFileName,
        EnumCallback<TranslationUnit*, bool>*
      ) = 0;
};


#ifdef DEBUG
// Assert that callers of methods returning new-ed objects
// wrap the returned pointers into RefPtr smart pointers
 #define get_return_symbol(t,s) \
    get_return_symbol((t),(s),&RefPtrEnforcer(__FILE__, __LINE__))
#else
 #define get_return_symbol(t,s) get_return_symbol((t),(s),0)
#endif

#endif // DEBUG_SYM_H__A7F13D4A_8183_455B_9739_BEB0D033A09F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
