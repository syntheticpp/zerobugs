#ifndef SLICER_H__567067D9_C0C9_416C_8B5F_5132ACDF82F9
#define SLICER_H__567067D9_C0C9_416C_8B5F_5132ACDF82F9
//
// $Id: slicer.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/debug_sym.h"
#include "zdk/zobject_impl.h"


CLASS Slicer : private ZObjectImpl<DebugSymbolCallback>
{
public:
    explicit Slicer(RefPtr<DebugSymbol> sym);

    void run(uint64_t low, uint64_t high, RefPtr<DebugSymbol> dest);

private:
    bool notify(DebugSymbol*);

    RefPtr<DebugSymbol> sym_;
    RefPtr<DebugSymbol> dest_;
    uint64_t low_, high_, current_;
};
#endif // SLICER_H__567067D9_C0C9_416C_8B5F_5132ACDF82F9
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
