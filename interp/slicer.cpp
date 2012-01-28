// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include <cassert>
#include <boost/limits.hpp>
#include "slicer.h"


using namespace std;


Slicer::Slicer(RefPtr<DebugSymbol> sym)
    : sym_(sym)
    , low_(0)
    , high_(numeric_limits<uint64_t>::max())
    , current_(0)
{
    assert(sym_);
}


void
Slicer::run(uint64_t low, uint64_t high, RefPtr<DebugSymbol> dest)
{
    assert(dest);

    low_ = low;

    if (high >= low)
    {
        high_ = high;
    }
    else
    {
        high_ = numeric_limits<uint64_t>::max();
    }
    current_ = 0;

    dest_ = dest;

    sym_->enum_children(this);
}


bool Slicer::notify(DebugSymbol* child)
{
    if ((current_ >= low_) && (current_ <= high_))
    {
        dest_->add_child(child);
    }
    ++current_;
    return true;
}



// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
