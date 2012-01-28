#ifndef DEBUG_SYMBOL_VECTOR_H__261F4114_1D3E_4E23_B2F3_A7656E9AFB9B
#define DEBUG_SYMBOL_VECTOR_H__261F4114_1D3E_4E23_B2F3_A7656E9AFB9B
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
#include <vector>
#include "zdk/weak_ptr.h"
#include "typez/public/debug_symbol_collection.h"
#include "typez/public/debug_symbol_impl.h"


/**
 * Implementation of the DebugSymbolCollection interface,
 * uses a vector internally
 */
class DebugSymbolVector : public DebugSymbolCollection
{
public:
    typedef std::vector<RefPtr<DebugSymbolImpl> > Vector;

    DebugSymbolVector();
    DebugSymbolVector(const DebugSymbolVector&);

    ~DebugSymbolVector() throw();

    size_t enumerate(DebugSymbolCallback* = NULL) const;

    void add(const RefPtr<DebugSymbolImpl>& sym);

    DebugSymbolCollection* clone() const;

    void set_method(Method*);

    RefPtr<Method> method() const;

    void detach_from_parent();

    DebugSymbol* nth_child(DebugSymbol&, size_t);

    void clear() { v_.clear(); }

    bool empty() const { return v_.empty(); }
    size_t size() const { return v_.size(); }

private:
    Vector v_;

    WeakPtr<Method> method_;
};

#endif // DEBUG_SYMBOL_VECTOR_H__261F4114_1D3E_4E23_B2F3_A7656E9AFB9B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
