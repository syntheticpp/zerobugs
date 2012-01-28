#ifndef DEBUG_REG_H__D19E9BA1_1F3B_4E9D_A4C8_99EA866B2B2B
#define DEBUG_REG_H__D19E9BA1_1F3B_4E9D_A4C8_99EA866B2B2B
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

#include "zdk/platform.h"
#include "zdk/unknown2.h"

using Platform::addr_t;
using Platform::reg_t;


/**
 * Interface for manipulating the debug registers.
 * @note modeled after the x86 hardware
 */
DECLARE_ZDK_INTERFACE_(DebugRegs, struct Unknown)
{
    enum Length
    {
        BREAK_ONE_BYTE  = 0,
        BREAK_TWO_BYTE  = 1,
        BREAK_FOUR_BYTE = 3
    };

    enum Condition
    {
        BREAK_ON_ANY             =-1,
        BREAK_ON_INSTRUCTION     = 0,
        BREAK_ON_DATA_WRITE      = 1,
        BREAK_ON_DATA_READ_WRITE = 3
    };

    /**
     * @return the number of debug registers supported by
     * the architecture (4 on x86)
     */
    virtual size_t max_size(Condition) const = 0;

    /**
     * Sets a hardware break at the specified address.
     * @param addr address (can be code or data)
     * @param regIndex if not NULL, and the function
     *  returns successfully, it will be filled out with
     *  the index of the debug register that is used for
     *  to hold the address.
     * @param global if true, the hardware breakpoint
     *  applies to all threads.
     * @param condition what triggers the breakpoint
     * @param bytelen length in bytes of watched area
     * @return true if successful.
     */
    virtual bool set_breakpoint(
        addr_t      addr,
        uint32_t*   regIndex,
        bool        global = false,
        Condition   cond = BREAK_ON_INSTRUCTION,
        Length      byteLen = BREAK_ONE_BYTE) = 0;

    /**
     * Enable/disable hardware breakpoint.
     * @param regIndex which debug register? (0 thru max_size())
     * @param activate if true, enable; otherwise disable
     * @param global applies to all threads?
     * @param condition what triggers the breakpoint
     * @param bytelen length in bytes of watched area
     */
    virtual void enable(
        uint32_t    regIndex,
        bool        activate,
        bool        global,
        Condition   cond,
        Length      length) = 0;

    /**
     * Permanently remove breakpoint at specified index.
     */
    virtual void clear(reg_t index) = 0;

    /**
     * Just a dorky name for operator[]
     * __attribute__((com_interface)) seems to have a problem
     * with operators inside COM-like interfaces.
     */
    virtual addr_t addr_in_reg(reg_t) const = 0;

    /**
     * If a hardware breakpoint was hit, return its address,
     * or zero otherwise. If the conditionOut parameter is not
     * NULL, fill it out with the condition.
     * @note the read bit in the status register (DR6)
     * is reset by this operation.
     */
    virtual addr_t hit(reg_t* conditionOut = 0) = 0;

};

#endif // DEBUG_REG_H__D19E9BA1_1F3B_4E9D_A4C8_99EA866B2B2B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
