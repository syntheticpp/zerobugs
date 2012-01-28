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
#include "zdk/symbol_table.h"
#include "private/offset_symbol.h"


OffsetSymbolImpl::OffsetSymbolImpl(Symbol& parent, addr_t offset)
    : parent_(&parent)
    , offset_(offset)
{
    assert(parent_->offset() == 0);
}


OffsetSymbolImpl::~OffsetSymbolImpl() throw()
{
}


/**
 * @return the symbol table containing this symbol
 */
SymbolTable* OffsetSymbolImpl::table(ZObjectManager* mgr) const
{
    return parent_->table(mgr);
}


/**
 * @return the lower bound address in the symbol table.
 * @see Symbol::offset()
 */
addr_t OffsetSymbolImpl::value() const
{
    return parent_->value();
}


/**
 * @return offset from the symbol's table address
 * @note this is used when symbols wrap an arbitrary
 * address in the text segment, rather than a
 * routine entry point.
 */
addr_t OffsetSymbolImpl::offset() const
{
    return offset_;
}


/**
 * @return the (possibly mangled) symbol name
 */
SharedString* OffsetSymbolImpl::name() const
{
    return parent_->name();
}


/**
 * @return the demangled function name (for C++),
 * optionally including the formal parameters.
 */
SharedString* OffsetSymbolImpl::demangled_name(bool params) const
{
    return parent_->demangled_name(params);
}


SharedString* OffsetSymbolImpl::demangle(bool params) const
{
    return parent_->demangle(params);
}


bool OffsetSymbolImpl::is_function() const
{
    return parent_->is_function();
}


void
OffsetSymbolImpl::set_deferred_breakpoint(BreakPoint::Type type,
                                          Runnable* runnable,
                                          BreakPoint::Action* action,
                                          Symbol* reserved)
{
    assert(reserved == 0);
    parent_->set_deferred_breakpoint(type, runnable, action, this);
}



// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
