#ifndef DEBUG_REGS_386_H__8E539174_745B_4B64_BD9D_11579A29553D
#define DEBUG_REGS_386_H__8E539174_745B_4B64_BD9D_11579A29553D
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

class Thread;


class ZDK_LOCAL DebugRegs386 : public DebugRegsBase
{
public:
    enum { DEBUG_REG_NUM = 4 };

    explicit DebugRegs386(Thread&);

    const Thread& thread() const { return thread_; }

    size_t max_size(Condition) const { return DEBUG_REG_NUM; }

    virtual bool set_breakpoint(
        addr_t,
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
     * Returns the currently hit address -- if any,
     * or null. This method resets DR6
     */
    virtual addr_t hit(reg_t*);

    /**
     * @return the contents of the status register (DR6)
     */
    /* virtual */ reg_t status() const;

    /**
     * @return the contents of the control register (DR7)
     */
    /* virtual */ reg_t control() const;

    void dump(std::ostream&) const;

private:
    void enable_internal(uint32_t, bool, bool, Condition, Length);

    Thread& thread_;
    addr_t  reg_[DEBUG_REG_NUM];
    reg_t   control_;
};

#endif // DEBUG_REGS_386_H__8E539174_745B_4B64_BD9D_11579A29553D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
