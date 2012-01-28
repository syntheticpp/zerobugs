#ifndef DEBUG_REGS_PPC_H__6C459717_7FB8_4C50_9AF9_C4899F2B342D
#define DEBUG_REGS_PPC_H__6C459717_7FB8_4C50_9AF9_C4899F2B342D
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
//
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
#include "debug_regs_base.h"


/**
 * Low-level support for Power PC hardware breakpoints
 */
class ZDK_LOCAL DebugRegsPPC : public DebugRegsBase
{
public:
    explicit DebugRegsPPC(Thread&);

    /**
     * thread that owns the debug regs
     */
    const Thread& thread() const { return thread_; }

    /**
     * @return the maximum number of registers supported
     * for the specified break condition
     */
    size_t max_size(Condition) const;

    virtual bool set_breakpoint(addr_t,
                                uint32_t*,
                                bool global,
                                Condition = BREAK_ON_INSTRUCTION,
                                Length = BREAK_ONE_BYTE);

    /**
     * Clear debug register at given index, and
     * all its associated status and control bits.
     */
    virtual void clear(reg_t index);

    virtual void enable(
        uint32_t    index,
        bool        onOff,
        bool        global,
        Condition = BREAK_ON_INSTRUCTION,
        Length = BREAK_ONE_BYTE);

    virtual addr_t addr_in_reg(reg_t) const;

    /**
     * Returns the currently hit address, or zero
     */
    virtual addr_t hit(reg_t* conditionOut = NULL);

    void dump(std::ostream&) const;

private:
    bool find_available_dabr(int&) const;
    bool find_available_iabr(int&) const;

private:
    Thread& thread_;

    static const unsigned long MAX_REG = 32;
    addr_t watchAddr_[32]; // addresses to watch for, first 16
                           // are DABR, followed by 16 IABR
};

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
#endif // DEBUG_REGS_PPC_H__6C459717_7FB8_4C50_9AF9_C4899F2B342D
