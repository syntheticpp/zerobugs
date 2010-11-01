#ifndef MEMIO_H__BBE7DA66_57A3_11DA_B82B_00C04F09BBCC
#define MEMIO_H__BBE7DA66_57A3_11DA_B82B_00C04F09BBCC
//
// $Id: memio.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sys/types.h>
#include "zdk/platform.h"
#include "zdk/zobject.h"

using Platform::addr_t;
using Platform::word_t;


DECLARE_ZDK_INTERFACE_(MemoryIO, ZObject)
{
    DECLARE_UUID("f7d89d94-57a3-11da-b82b-00c04f09bbcc")

    /**
     * Read an area from the debugged process' memory into
     * a buffer of machine words. The third parameter gives
     * the size of the buffer, in machine words.
     * Optionally return the number of machine words read.
     * @note do not mistake words (32-bit on IA32) for ushorts
     */
    virtual void read_data(
        addr_t  address,
        word_t* buffer,
        size_t  howManyWords,
        size_t* wordsRead = 0) const = 0;

    virtual void write_data(addr_t, const word_t*, size_t) = 0;

    virtual void read_code(
        addr_t  address,
        word_t* buffer,
        size_t  howManyWords,
        size_t* wordsRead = 0) const = 0;

    virtual void write_code(addr_t, const word_t*, size_t) = 0;
};

#endif // MEMIO_H__BBE7DA66_57A3_11DA_B82B_00C04F09BBCC
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
