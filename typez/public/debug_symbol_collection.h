#ifndef DEBUG_SYMBOL_COLLECTION_H__2C8E8816_76E9_4E44_9B34_73B3FC927E11
#define DEBUG_SYMBOL_COLLECTION_H__2C8E8816_76E9_4E44_9B34_73B3FC927E11
//
// $Id: debug_symbol_collection.h 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include "zdk/debug_sym.h"
#include "zdk/types.h"  // for the Method interface definition

class DebugSymbolImpl;


struct DebugSymbolCollection
{
    virtual ~DebugSymbolCollection() { }

    virtual void add(const RefPtr<DebugSymbolImpl>&) = 0;

    virtual size_t enumerate(DebugSymbolCallback* = 0) const = 0;

    virtual DebugSymbolCollection* clone() const = 0;

    virtual void set_method(Method*) = 0;
    virtual RefPtr<Method> method() const = 0;

    virtual DebugSymbol* nth_child(DebugSymbol&, size_t n) = 0;

    virtual void detach_from_parent() = 0;

    virtual void clear() = 0;
};

#endif // DEBUG_SYMBOL_COLLECTION_H__2C8E8816_76E9_4E44_9B34_73B3FC927E11
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
