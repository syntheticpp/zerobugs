//
// $Id: collect_symbol.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <iostream>
#include "zdk/shared_string_impl.h"
#include "zdk/type_system.h"
#include "typez/public/debug_symbol_impl.h"
#include "typez/public/types.h"
#include "collect_symbol.h"


static void add_overload
(
    TypeSystem&             types,
    RefPtr<DebugSymbol>&    debugSym,
    DebugSymbol&            sym,
    Method*                 method
)
{
    assert(interface_cast<FunType*>(debugSym->type()));
    // a hack to deal with overloaded functions:
    // make a bogus function type and make all the
    // overloads children of debugSym
    RefPtr<SharedString> fname = shared_string("overloaded");
    RefPtr<DataType> type = new FunTypeImpl(types, *fname);

    std::string name;
    if (sym.name())
    {
        name.assign(sym.name()->c_str());
    }
    else
    {
        name.assign(fname->c_str());
    }
    assert(debugSym.get());
    assert(debugSym->enum_children() == 0);
    assert(sym.thread());
    assert(sym.thread() == debugSym->thread());

    RefPtr<DebugSymbolImpl> tmp = DebugSymbolImpl::create(
        *debugSym->thread(),
        *type,
        name,
        sym.name(),
        debugSym->reader(),
        method);

    assert(tmp->is_constant());
    tmp->add_child(debugSym.get());
    if (size_t nChildren = sym.enum_children())
    {
        for (size_t i = 0; i != nChildren; ++i)
        {
            tmp->add_child(sym.nth_child(i));
        }
    }
    else if (debugSym.get() != &sym)
    {
        tmp->add_child(&sym);
    }
    debugSym = tmp;
}


#ifdef DEBUG

static void dump_symbol(std::ostream& os, const DebugSymbol* sym)
{
    if (!sym)
    {
        os << "(null symbol)";
    }
    else
    {
        os << sym->name() << ": " << sym->type()->name();
        os << " [" << sym->enum_children() << "]";
    }
    os << std::endl;
}
#endif


/**
 * Make sym a child symbol of debugSym if the latter is non-null,
 * otherwise assign sym to debugSym.
 * This is useful when enumerating symbols and serveral matches
 * are found for a given name.
 */
void collect_symbol
(
    TypeSystem&             types,
    RefPtr<DebugSymbol>&    debugSym,
    DebugSymbol&            sym,
    Method*                 method
)
{
    DebugSymbolImpl& impl = interface_cast<DebugSymbolImpl&>(sym);
    impl.set_method(method);

    assert(sym.addr() || sym.enum_children());

    // dump_symbol(std::clog << "parent: ", debugSym.get());
    // dump_symbol(std::clog << "symbol: ", &sym);

    if (!debugSym)
    {
        debugSym = &sym;
    }
    else if (debugSym->is_constant())
    {
        debugSym->add_child(&sym);
    }
    else if (debugSym->addr() != sym.addr())
    {
        add_overload(types, debugSym, sym, method);
    }
#ifdef DEBUG
    else
    {
        dump_symbol(std::clog << "Parent: ", debugSym.get());
        dump_symbol(std::clog << "Symbol: ", &sym);
    }
#endif
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
