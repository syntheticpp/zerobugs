// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: block.cpp 714 2010-10-17 10:03:52Z root $
//
#include "block.h"

using Platform::addr_t;

HeapBlockImpl::HeapBlockImpl(
    addr_t addr,
    size_t size,
    const RefPtr<StackTrace>& trace)
 : addr_(addr)
 , size_(size)
 , free_(false)
 , trace_(trace)
{
}


HeapBlockImpl::~HeapBlockImpl() throw()
{
}


void HeapBlockImpl::mark_as_free()
{
    assert(!free_);
    free_ = true;

    trace_.reset();
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
