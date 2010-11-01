#ifndef FRAME_SETUP_H__1A7510A4_4C1D_4AF9_ADD4_1D8F1BD67687
#define FRAME_SETUP_H__1A7510A4_4C1D_4AF9_ADD4_1D8F1BD67687
//
// $Id: frame_setup.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/export.h"
#include "zdk/platform.h"

using Platform::addr_t;

class Thread;
class Variant;

/**
 * Sets up the stack frame of the debugged process/thread
 * when about to call a function from the builtin interpreter
 */
class ZDK_LOCAL FrameSetup
{
public:
    FrameSetup() : redZoneSet_(false), stackPtr_(0)
    { }

    /**
     * Push a function argument onto the given thread's stack
     * @return the current stack pointer where additional
     * arguments can be pushed
     * @note does not necessarily update the Thread's stack
     * pointer, it is the caller's responsibility to keep up
     * with the stack pointer changes.
     */
    addr_t push_arg(Thread& thread,
                    const Variant& arg,
                    addr_t stackPtr);

    RefPtr<Variant> push_literal(Thread&, DebugSymbol*, addr_t&);

    /**
     * Reserve a "red zone" on platforms that require it,
     * such as the AMD64 / GCC
     */
    void set_red_zone(Thread& thread, addr_t& stackPtr);

    addr_t stack_pointer() const { return stackPtr_; }

    addr_t reserve_stack(Thread&, size_t);

private:
    bool redZoneSet_;

    addr_t stackPtr_;
};

#endif // FRAME_SETUP_H__1A7510A4_4C1D_4AF9_ADD4_1D8F1BD67687
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
