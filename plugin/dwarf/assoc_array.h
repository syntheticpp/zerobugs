#ifndef ASSOC_ARRAY_H__B9D7C402_7760_4A9B_8690_C11F00FBA9E3
#define ASSOC_ARRAY_H__B9D7C402_7760_4A9B_8690_C11F00FBA9E3
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
#include "zdk/container.h"
#include "zdk/export.h"
#include "typez/public/data_type.h"

class DebugSymbolCollection;
class DebugSymbolImpl;

/**
 * D Associative Array (experimental)
 *
 * @note this is not necessarily the best place for this class;
 * it may be cleaner to move all D-specific type support into a
 * separate shared lib. It illustrates however the point that new
 * data types can be added by plug-ins.
 *
 */
class ZDK_LOCAL AssociativeArray
    : public DataTypeImpl<AssociativeContainerType>
{
    typedef DataTypeImpl<AssociativeContainerType> BaseType;

public:
    DECLARE_UUID("441cfc04-6273-415a-9dea-8c2f45b97efb")

BEGIN_INTERFACE_MAP(AssociativeArray)
    INTERFACE_ENTRY(AssociativeArray)
    INTERFACE_ENTRY_INHERIT(BaseType);
END_INTERFACE_MAP()

    AssociativeArray(TypeSystem&,
                     DataType& keyType,
                     DataType& elemType);

    ///// DataType interface /////
    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    bool is_equal(const DataType*) const;

    bool is_fundamental() const { return false; }

    size_t parse(const char*, Unknown2*) const;

    ///// ContainerType interface //////
    /**
     * @return the first child element
     */
    virtual DebugSymbol* first(DebugSymbol* parent) const;

    virtual DebugSymbol* next( DebugSymbol* parent,
                               DebugSymbol* child) const;

    /**
     * @return current number of elements in container object
     */
    virtual size_t count(DebugSymbol* container) const;

    virtual DataType* elem_type() const;

    ///// AssociativeContainerType interface //////
    virtual DataType* key_type() const ;

    virtual size_t enum_by_key( DebugSymbol* parent,
                                DebugSymbol* key,
                                DebugSymbolCallback*) const;
private:
    bool walk_tree(DebugSymbolImpl* array,
                   DebugSymbolEvents* events,
                   addr_t addr,
                   Thread& thread,
                   DataType& keyType,
                   DataType& valType,
                   DebugSymbolCollection&) const;
    /**
     * Advance the pointer a given number of machine words.
     */
    void advance_ptr(addr_t& addr, size_t n = 1) const
    {
        addr += (n * wordSize_) / Platform::byte_size;
    }

    WeakDataTypePtr keyType_,
                    valType_;

    const int wordSize_;
    mutable size_t count_;
};

#endif // ASSOC_ARRAY_H__B9D7C402_7760_4A9B_8690_C11F00FBA9E3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
