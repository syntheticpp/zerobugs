#ifndef CONTAINER_H__69F3C5AA_5B6F_435A_87F0_D8F24CADD063
#define CONTAINER_H__69F3C5AA_5B6F_435A_87F0_D8F24CADD063
//
// $Id: container.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/data_type.h"


/**
 * Generic container type
 */
DECLARE_ZDK_INTERFACE_(ContainerType, DataType)
{
    DECLARE_UUID("0456c69d-f253-4db2-bff1-f72c0bf9d084")

    /**
     * @return the first child element
     */
    virtual DebugSymbol* first(DebugSymbol* parent) const = 0;

    virtual DebugSymbol* next( DebugSymbol* parent,
                               DebugSymbol* child) const = 0;

    /**
     * @return current number of elements in container object
     */
    virtual size_t count(DebugSymbol* container) const = 0;

    virtual DataType* elem_type() const = 0;
};



DECLARE_ZDK_INTERFACE_(AssociativeContainerType, ContainerType)
{
    DECLARE_UUID("23713449-5564-44ee-9894-bbdafb0fc06c")

    virtual DataType* key_type() const = 0;

    virtual size_t enum_by_key( DebugSymbol* parent,
                                DebugSymbol* key,
                                DebugSymbolCallback*) const = 0;
};

#endif // CONTAINER_H__69F3C5AA_5B6F_435A_87F0_D8F24CADD063
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
