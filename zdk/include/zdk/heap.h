#ifndef HEAP_H__E3002450_D8F9_4D03_94BC_51C7CE037F77
#define HEAP_H__E3002450_D8F9_4D03_94BC_51C7CE037F77
//
// $Id: heap.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/enum.h"
#include "zdk/platform.h"
#include "zdk/stack.h"
#include "zdk/zobject.h"


/**
 * Abstract interface -- models a heap block.
 */
DECLARE_ZDK_INTERFACE_(HeapBlock, ZObject)
{
    DECLARE_UUID("335724c2-3c1e-485c-bcdf-c59ec0bfdb33")

    virtual Platform::addr_t addr() const = 0;

    virtual size_t size() const = 0;

    virtual StackTrace* stack_trace() = 0;

    /* todo: pid, pthread_id? */
};



DECLARE_ZDK_INTERFACE_(Heap, Unknown)
{
    DECLARE_UUID("b5b9352a-237e-4013-8ba4-58f54c42ab07")

    enum EnumOption
    {
        NONE = 0,
        SORT_SIZE_INCREASING,
        SORT_SIZE_DECREASING,
    };

    /**
     * Calls the enum callback sink's (if not NULL) notify method
     * for each and every heap block currently in use. Returns
     * the number of heap blocks in use.
     */
    virtual size_t enum_blocks(
            EnumCallback<const HeapBlock*>*,
            EnumOption = NONE) = 0;

    /**
     * Returns the total number of bytes allocated on the heap.
     */
    virtual size_t total() const = 0;
};

#endif // HEAP_H__E3002450_D8F9_4D03_94BC_51C7CE037F77
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
