#ifndef SYMBOL_ADDR_H__D19230AE_DFE1_49E2_BE4F_ACBC33D49E32
#define SYMBOL_ADDR_H__D19230AE_DFE1_49E2_BE4F_ACBC33D49E32
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "zdk/symbol.h"
#include "zdk/symbol_table.h"
#include "zdk/zobject_scope.h"
#include "private/debug.h"

using namespace std;

/**
 * The symbol address can be calculated in terms of other
 * public methods, I could expose this function instead
 * of Symbol::addr, but I find the latter more convenient.
 */
inline ZDK_LOCAL addr_t symbol_addr(const Symbol& sym)
{
    addr_t addr = sym.value();
    if (addr)
    {
        addr += sym.offset();
        ZObjectScope _scope_;
        if (RefPtr<SymbolTable> tbl = sym.table(&_scope_))
        {
            if (tbl->addr())
            {
                addr += tbl->adjustment();
            }
        }
    }
    return addr;
}


inline ZDK_LOCAL addr_t
get_relative_addr(const SymbolTable& table, addr_t addr)
{
    if (table.addr() == 0)
    {
        IF_DEBUG
        (
            std::cerr << "addr is 0, adjustment="  << std::hex
                      << table.adjustment() << " in " << table.filename()
                      << std::dec << std::endl;
        )
    }
    else
    {
        addr -= table.adjustment();
    }
    return addr;
}
#endif // SYMBOL_ADDR_H__D19230AE_DFE1_49E2_BE4F_ACBC33D49E32
