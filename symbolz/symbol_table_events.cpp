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
#include "private/debug.h"
#include "public/symbol_table_events.h"


using namespace std;


bool SymbolTableEvents::use_lazy_loading() const
{
    return false;
}


bool SymbolTableEvents::on_symbol(SymbolTable&, Symbol& symbol)
{
    const bool keep = symbol.value() && *symbol.name()->c_str();

    return keep;
}


size_t SymbolTableEvents::addr_to_line( const SymbolTable&,
                                        addr_t,
                                        RefPtr<SharedString>&)
{
    return 0; // place-holder
}


size_t
SymbolTableEvents::line_to_addr(const SymbolTable&,
                                RefPtr<SharedString>,
                                size_t,
                                EnumCallback<addr_t>*)
{
    return 0; // place-holder
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
