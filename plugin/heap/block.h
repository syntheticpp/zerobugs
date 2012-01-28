#ifndef BLOCK_H__70A6672C_B682_4A81_AB84_ADBD2F3DE513
#define BLOCK_H__70A6672C_B682_4A81_AB84_ADBD2F3DE513
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

#include "zdk/heap.h"
#include "zdk/stack.h"
#include "zdk/zobject_impl.h"


class HeapBlockImpl : public ZObjectImpl<HeapBlock>
{
public:
    HeapBlockImpl(Platform::addr_t, size_t, const RefPtr<StackTrace>&);

    ~HeapBlockImpl() throw();

    void mark_as_free();

    bool is_free() const { return free_; }

    Platform::addr_t addr() const { return addr_; }

    size_t size() const { return size_; }

    StackTrace* stack_trace() { return trace_.get(); }

private:
    Platform::addr_t    addr_;
    size_t              size_ : sizeof(size_t) * 8 - 1;
    bool                free_ : 1;
    RefPtr<StackTrace>  trace_;
};
#endif // BLOCK_H__70A6672C_B682_4A81_AB84_ADBD2F3DE513
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
