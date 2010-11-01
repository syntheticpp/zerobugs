#ifndef ADDR_OPS_H__1211D4CB_41F3_44AD_B5BD_BB967A252FD5
#define ADDR_OPS_H__1211D4CB_41F3_44AD_B5BD_BB967A252FD5
//
// $Id: addr_ops.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <libdwarf.h>

namespace Dwarf
{
    /* Some addressing operations may need
       external help in order to evaluate. The
       client code needs to specify methods for
       reading from the CPU registers and
       from memory locations */

    /**
     * Interface that the Locations object may call
     * when evaluating an address. The client code
     * may provide its own implementation and call
     * set_addr_ops()
     */
    struct AddrOps
    {
        virtual ~AddrOps() {}

        virtual Dwarf_Addr read_mem(Dwarf_Addr) = 0;
        virtual Dwarf_Addr read_cpu_reg(Dwarf_Signed, bool = true) = 0;

        virtual bool is_32_bit() const = 0;
    };


    /* Set/get the AddrOps callback object.
       NOTE: These function is not thread-safe.
       The client code is responsible for ensuring
       thread safety. */

    AddrOps* set_addr_operations(AddrOps*);

    AddrOps* get_addr_operations();
}
#endif // ADDR_OPS_H__1211D4CB_41F3_44AD_B5BD_BB967A252FD5
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
