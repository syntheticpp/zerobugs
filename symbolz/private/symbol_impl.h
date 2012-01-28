#ifndef SYMBOL_IMPL_H__3DB44694_18AB_4B8C_8163_955DEAA98BCC
#define SYMBOL_IMPL_H__3DB44694_18AB_4B8C_8163_955DEAA98BCC
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
#include "zdk/fobject.h"
#include "zdk/ref_counted_impl.h"
#include "zdk/ref_ptr.h"
#include "zdk/symbol.h"
#include "zdk/weak_ptr.h"
#include "dharma/object_manager.h"

using Platform::addr_t;

class Process;
class SymbolTableImpl;
namespace ELF { class Symbol; }


/**
 * Base for SymbolImpl and OffsetSymbolImpl
 */
CLASS BaseSymbolImpl : public Symbol, public Fobject<16>
{
public:
    DECLARE_UUID("626b61ee-a03e-49cf-8190-4b2788b43870")

    BEGIN_INTERFACE_MAP(BaseSymbolImpl)
        INTERFACE_ENTRY(Symbol)
        INTERFACE_ENTRY(BaseSymbolImpl)
    END_INTERFACE_MAP()

    virtual addr_t addr() const;

protected:
    typedef RefPtr<SharedString> StringPtr;

    BaseSymbolImpl() : data_(0) { }
    virtual ~BaseSymbolImpl() { }

    int8_t type() const { return type_; }

    //NOT USED
    //int8_t binding() const { return binding_; }

private:
    BaseSymbolImpl(const BaseSymbolImpl&);
    BaseSymbolImpl& operator=(const BaseSymbolImpl&);

    /**
     * @return the source file name (if known, or the
     * name of the binary that contains it otherwise)
     */
    virtual SharedString* file() const;

    /**
     * @return the line number in source file
     */
    virtual uint32_t line() const;

    void trim_memory() { }

protected:
    void get_source_line() const;

    mutable StringPtr file_;   // source file, or module name
    union
    {
        struct
        {
            mutable uint32_t    line_ : 22;         // source line
            mutable bool        hasSourceLine_ : 1;
            mutable bool        hasParam_ : 1;      // used by derived class,
                                                    //  decl here for tight packing
            uint8_t             type_ : 4;          // STB_HIPROC == 15
            uint8_t             binding_ : 4;       // STT_HIPROC == 15
        };
        uint32_t data_;
    };
};


/**
 * A symbol that has a corresponding entry in the symbol table
 */
CLASS SymbolImpl
    : public RefCountedImpl<NoWeakRef<BaseSymbolImpl> >
#ifdef DEBUG_OBJECT_LEAKS
     public CountedInstance<SymbolImpl>
#endif
{
protected:
    typedef WeakPtr<SymbolTable> SymbolTablePtr;

public:
    DECLARE_UUID("71f0e9bc-46cb-4079-872b-998844885adf")

    BEGIN_INTERFACE_MAP(SymbolImpl)
        INTERFACE_ENTRY_INHERIT(BaseSymbolImpl)
    END_INTERFACE_MAP()

    SymbolImpl( SymbolTableImpl&,
                const ELF::Symbol&,
                const RefPtr<SharedString>&);

    SymbolImpl( const SymbolTable&,
                const RefPtr<SharedString>&,
                addr_t value,
                int type);

    virtual ~SymbolImpl() throw();

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

    void set_deferred_breakpoint(BreakPoint::Type,
                                 Runnable*,
                                 BreakPoint::Action*,
                                 Symbol*);
    void trim_memory();

private:
    SymbolTablePtr      table_;
    StringPtr           name_;
    addr_t              value_;
    mutable StringPtr   demangledName_;
};


#endif // SYMBOL_IMPL_H__3DB44694_18AB_4B8C_8163_955DEAA98BCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
