#ifndef COMPLEX_H__F298E77F_8433_4805_91B8_39DF2C9E58A1
#define COMPLEX_H__F298E77F_8433_4805_91B8_39DF2C9E58A1
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: complex.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "typez/public/data_type_impl.h"


/**
 * C99 _Complex
 */
class ZDK_LOCAL ComplexImpl : public DataTypeImpl<ComplexType>
{
    typedef DataTypeImpl<ComplexType> BaseType;

    // the type for the real and imaginary coordinates
    RefPtr<DataType> partType_;

protected:
    friend class NativeTypeSystem;

    ComplexImpl(SharedString*, DataType& type);

public:
    DECLARE_UUID("9b1f644c-37f8-4516-8f11-eb70d1c9c0ed")

BEGIN_INTERFACE_MAP(ComplexImpl)
    INTERFACE_ENTRY(ComplexImpl)
    INTERFACE_ENTRY_INHERIT(BaseType)
END_INTERFACE_MAP()

private:
    DataType* part_type() const { return partType_.get(); }

    bool is_fundamental() const { return true; }

    int compare(const char*, const char*) const;

    SharedString* read(DebugSymbol*, DebugSymbolEvents*) const;

    void write(DebugSymbol*, const Buffer*) const;

    bool is_equal(const DataType*) const;

    size_t parse(const char*, Unknown2*) const;
};

#endif // COMPLEX_H__F298E77F_8433_4805_91B8_39DF2C9E58A1
