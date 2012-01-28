#ifndef DYNAMIC_ARRAY_H__7D044097_7300_4A1C_B420_F6ED0B0BC643
#define DYNAMIC_ARRAY_H__7D044097_7300_4A1C_B420_F6ED0B0BC643
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

#include "zdk/export.h"
#include "zdk/types.h"
#include "typez/public/data_type_impl.h"


/**
 * Implements D-language's dynamic array type
 */
class ZDK_LOCAL DynamicArray
    : public DataTypeImpl<DynamicArrayType>
{
    typedef DataTypeImpl<DynamicArrayType> BaseType;

public:
    DECLARE_UUID("3816f4e9-b318-48ba-ba83-37ee386dc783")

BEGIN_INTERFACE_MAP(DynamicArray)
    INTERFACE_ENTRY(DynamicArray)
    INTERFACE_ENTRY_INHERIT(BaseType)
END_INTERFACE_MAP()

    DynamicArray(TypeSystem&, DataType&);

    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    //
    // todo: this should depend on the language: it is
    // true in D, but not true in C++
    //
    bool is_fundamental() const { return false; }

    bool is_equal(const DataType*) const;

    DataType* elem_type() const
    { return elemType_.ref_ptr().get(); }

    size_t elem_count() const;

    size_t parse(const char* str, Unknown2* unk) const;

    addr_t first_elem_addr(DebugSymbol*) const;

    size_t count(DebugSymbol*) const;

private:
    WeakDataTypePtr elemType_;
    const int wordSize_;
};

#endif // DYNAMIC_ARRAY_H__7D044097_7300_4A1C_B420_F6ED0B0BC643
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
