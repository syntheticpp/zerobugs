//
// $Id: symbol_util.cpp 710 2010-10-16 07:09:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iomanip>
#include <iostream>
#include "zdk/data_filter.h"
#include "zdk/zobject_scope.h"
#include "zdk/zero.h" // for Thread
#include "generic/state_saver.h"
#include "symbol_util.h"
#include "unmangle/unmangle.h"

using namespace std;


////////////////////////////////////////////////////////////////
void print_symbol(ostream& outs,
                  addr_t addr,
                  RefPtr<Symbol> sym,
                  bool showModule)
{
    outs << "0x" << hex;
    outs.setf(ios::right);

    outs << setfill('0') << setw(2 * sizeof(addr_t)) << addr << dec;
    outs << ' ';

    if (showModule)
    {
        ZObjectScope scope;
        if (sym && sym->table(&scope))
        {
            outs << "in " << sym->table(&scope)->filename() << ": ";
        }
    }
    if (!sym)
    {
        outs << "???";
    }
    else
    {
        StateSaver<ios, ios::fmtflags> ifs(outs);
        outs << sym->file()->c_str();

        if (sym->line())
        {
            outs << ':' << sym->line();
        }
        if (addr_t offs = sym->offset())
        {
            outs << " <" << sym->demangled_name(false);

            // showbase not always available, print 0x explicitly
            outs << " +0x"<< hex << offs << dec << '>';
        }
        else
        {
            outs << " " << sym->demangled_name(false);
        }
    }
}


void print_symbol(ostream& outs, const Symbol& sym)
{
    const addr_t addr = sym.addr();

    outs << "0x" << hex;
    outs.setf(ios::right);

    outs << setfill('0') << setw(2 * sizeof(addr_t)) << addr << dec;
    outs << ' ';

    StateSaver<ios, ios::fmtflags> ifs(outs);
    outs << sym.file()->c_str();

    if (sym.line())
    {
        outs << ':' << sym.line();
    }
    if (addr_t offs = sym.offset())
    {
        outs << " <" << sym.demangled_name(false);

        // showbase not always available, print 0x explicitly
        outs << " +0x"<< hex << offs << dec << '>';
    }
    else
    {
        outs << " " << sym.demangled_name(false);
    }
}



RefPtr<DebugSymbol>
apply_transform(DebugSymbol& sym,
                DebugSymbol* parent,
                DebugSymbolEvents* events)
{
    if (Thread* thread = sym.thread())
    {
        if (DataFilter* flt = interface_cast<DataFilter*>(thread->debugger()))
        {
            if (DebugSymbol* s = flt->transform(&sym, parent, events))
            {
                return  s;
            }
        }
    }
    return &sym;
}


void SymbolEnum::notify(Symbol* symbol)
{
    assert(symbol);
    if (uniqueAddrs_.insert(symbol->addr()).second)
    {
        symbols_.push_back(RefPtr<Symbol>(symbol));
    }
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
