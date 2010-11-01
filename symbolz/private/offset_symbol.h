#ifndef OFFSET_SYMBOL_H__AFF3AB67_3F71_4BE8_A1F5_218C6B1C6427
#define OFFSET_SYMBOL_H__AFF3AB67_3F71_4BE8_A1F5_218C6B1C6427
//
// $Id: offset_symbol.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "symbol_impl.h"

/**
 * An "artificial" symbol -- corresponds to a code address
 * which is off by offset_ from the nearest entry in sym table.
 */
CLASS OffsetSymbolImpl
    : public RefCountedImpl<NoWeakRef<BaseSymbolImpl> >
#ifdef DEBUG_OBJECT_LEAKS
    , public CountedInstance<OffsetSymbolImpl>
#endif
{
public:
    BEGIN_INTERFACE_MAP(OffsetSymbolImpl)
        INTERFACE_ENTRY_INHERIT(BaseSymbolImpl)
    END_INTERFACE_MAP()

    OffsetSymbolImpl(Symbol& parent, addr_t offset);
    virtual ~OffsetSymbolImpl() throw();

    /**
     * @return the symbol table containing this symbol
     */
    virtual SymbolTable* table(ZObjectManager*) const;

    /**
     * @return the lower bound address in the symbol table.
     * @see Symbol::offset()
     */
    virtual addr_t value() const;

    /**
     * @return offset from the symbol's table address
     * @note this is used when symbols wrap an arbitrary
     * address in the text segment, rather than a
     * routine entry point.
     */
    virtual addr_t offset() const;

    /**
     * @return the (possibly mangled) symbol name
     */
    virtual SharedString* name() const;

    /**
     * @return the demangled function name (for C++),
     * optionally including the formal parameters.
     */
    virtual SharedString* demangled_name(bool params = true) const;

    virtual SharedString* demangle(bool params = true) const;

    virtual bool is_function() const;

    virtual void set_deferred_breakpoint(
        BreakPoint::Type,
        Runnable*,
        BreakPoint::Action*,
        Symbol*);

private:
    RefPtr<Symbol>  parent_;
    addr_t offset_;
};


#endif // OFFSET_SYMBOL_H__AFF3AB67_3F71_4BE8_A1F5_218C6B1C6427
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
