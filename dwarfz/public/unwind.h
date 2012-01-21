#ifndef UNWIND_H__CA29E1F9_2B23_4525_8FFB_7F6B94CC01D3
#define UNWIND_H__CA29E1F9_2B23_4525_8FFB_7F6B94CC01D3
//
// $Id: unwind.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <libdwarf.h>
#include "addr_ops.h"

namespace Dwarf
{
    struct RegTable
    {
        enum State
        {
            REG_NA,     // register value not available
            REG_READY,  // value is ready
            REG_PENDING // we're about to calculate the value
        };
        struct
        {
            State state;
            Dwarf_Addr value;
        } regs[DW_REG_TABLE_SIZE];

        RegTable();
    };

    /**
     * Frame Unwind Info
     */
    CLASS Unwind
    {
    public:
        explicit Unwind(Dwarf_Debug);

        ~Unwind() throw();

        /**
         * Unwind the stack trace, using .debug_frame info.
         * @return the return address if successful, zero otherwise.
         * @param pc starting program counter
         * @param addrOps an implementation of address space
         * operations, for reading from the debuggee's registers
         * and memory.
         * @param regs carries register values from one frame
         * to another; must be initialized by caller
         */
        Dwarf_Addr step(Dwarf_Addr pc, AddrOps& addrOps, RegTable& regs);

        /**
         * Compute the CFA at the local program count.
         * For supporting DW_OP_call_frame_cfa:
         */
        Dwarf_Addr compute_cfa(Dwarf_Addr pc, AddrOps& ops);

    private:
        Unwind(const Unwind&);
        Unwind& operator=(const Unwind&);

        Dwarf_Debug     dbg_;
        Dwarf_Cie*      cie_;
        Dwarf_Signed    cieElemCount_;
        Dwarf_Fde*      fde_;
        Dwarf_Signed    fdeElemCount_;
     // Dwarf_Regtable  regs_;
    };
}
#endif // UNWIND_H__CA29E1F9_2B23_4525_8FFB_7F6B94CC01D3
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
