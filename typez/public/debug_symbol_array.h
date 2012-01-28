#ifndef DEBUG_SYMBOL_ARRAY_H__6D7CC0E1_058E_4282_AAA9_9F54674A06E7
#define DEBUG_SYMBOL_ARRAY_H__6D7CC0E1_058E_4282_AAA9_9F54674A06E7
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
#include "typez/public/debug_symbol_vector.h"


class DebugSymbolArray : public DebugSymbolVector
{
    mutable bool sparse_;

private:
    DebugSymbolCollection* clone() const;

    DebugSymbol* nth_child(DebugSymbol&, size_t);

    void add(const RefPtr<DebugSymbolImpl>& sym);

    RefPtr<DebugSymbolImpl>
        add_element(DebugInfoReader* reader,
                    Thread& thread,
                    DebugSymbolImpl& array,
                    DataType& elemType,
                    DebugSymbolEvents* events,
                    addr_t addr,
                    uint64_t index,
                    uint64_t offset);

public:
    DebugSymbolArray() : sparse_(false)
    { }

    /**
     * Reads the child elements [0,upperElemIndex]
     */
    void read(  DebugInfoReader*,
                DebugSymbolImpl& array,
                const addr_t arrayAddr,
                uint64_t upperElemIndex,
                DataType& elemType,
                DebugSymbolEvents*,
                uint64_t offset = 0);

    /**
     * Reads child elements within the specified range, from
     * lowerElemIndex up to and including upperElemIndex.
     */
    void read(  DebugInfoReader*,
                DebugSymbolImpl& array,
                const addr_t arrayAddr,
                uint64_t lowerElemIndex,
                uint64_t upperElemIndex,
                DataType& elemType,
                DebugSymbolEvents*,
                uint64_t offset = 0);
};


#endif // DEBUG_SYMBOL_ARRAY_H__6D7CC0E1_058E_4282_AAA9_9F54674A06E7
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
