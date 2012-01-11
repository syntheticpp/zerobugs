//
// $Id: symbol.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <sstream>
#include "elfz/public/symbol.h"
#include "zdk/stdexcept.h"
#include "zdk/check_ptr.h"
#include "zdk/process.h"
#include "zdk/runnable.h"
#include "zdk/shared_string_impl.h"
#include "zdk/type_system.h"
#include "private/symbol_addr.h"
#include "private/symbol_impl.h"
#include "private/symbol_table_impl.h"
#include "unmangle/unmangle.h"

using namespace std;


SymbolImpl::~SymbolImpl() throw()
{
}


SymbolImpl::SymbolImpl
(
    SymbolTableImpl& table,
    const ELF::Symbol& sym,
    const RefPtr<SharedString>& name
)
    : table_(&table)
    , name_(name)
    , value_(sym.value())
{
    type_ = sym.type() & 0xf;

    //NOT USED
    //binding_ = sym.type() >> 4;
}


SymbolImpl::SymbolImpl
(
    const SymbolTable& table,
    const RefPtr<SharedString>& name,
    addr_t value,
    int type
)
    : table_(&table)
    , name_(name)
    , value_(value)
{
    type_ = type & 0xf;
}


SymbolTable* SymbolImpl::table(ZObjectManager* mgr) const
{
    if (RefPtr<SymbolTable> table =  table_.ref_ptr())
    {
        assert(mgr);
        if (mgr)
        {
            mgr->manage(table.get());
            return table.get();
        }
    }
    return NULL;
}


addr_t SymbolImpl::value() const
{
    return value_;
}


addr_t SymbolImpl::offset() const
{
    return 0;
}



SharedString* SymbolImpl::name() const
{
    assert(name_);
    return name_.get();
}


/**
 * This function guarantees to never return NULL
 */
SharedString* SymbolImpl::demangled_name(bool params) const
{
    if (SharedString* demangledName = this->demangle(params))
    {
        return demangledName;
    }
    assert(name_);
    return name_.get();
}



SharedString* SymbolImpl::demangle(bool params) const
{
    if (!demangledName_ || (params != hasParam_))
    {
        demangledName_.reset();

        const int flags = params ? UNMANGLE_DEFAULT : UNMANGLE_NOFUNARGS;
        size_t len = name()->length();

        if (char* str = unmangle(name()->c_str(), &len, 0, flags))
        {
            demangledName_ = SharedStringImpl::take_ownership(str);
        }
        hasParam_ = params;
    }
    return demangledName_.get();
}



SharedString* BaseSymbolImpl::file() const
{
    if (!hasSourceLine_)
    {
        get_source_line();
    }

    if (!file_)
    {
        ZObjectScope scope;
        if (SymbolTable* table = this->table(&scope))
        {
            file_ = table->filename();
        }
        else
        {
            file_ = SharedStringImpl::create();
        }
    }
    return file_.get();
}


uint32_t BaseSymbolImpl::line() const
{
    if (!hasSourceLine_)
    {
        get_source_line();
    }
    return line_;
}


void BaseSymbolImpl::get_source_line() const
{
    assert(line_ == 0);

    ZObjectScope scope;
    if (SymbolTable* tbl = this->table(&scope))
    {
        if (tbl->is_virtual_shared_object())
        {
            file_ = tbl->filename();
            hasSourceLine_ = true;
        }
        else if (SymbolTableEvents* events = interface_cast<SymbolTableEvents*>(tbl))
        {
            line_ = events->addr_to_line(*tbl, value() + offset(), file_);
            if (file_)
            {
                hasSourceLine_ = true;
            }
        }
    }
}


addr_t BaseSymbolImpl::addr() const
{
    return symbol_addr(*this);
}


bool SymbolImpl::is_function() const
{
    return type() == STT_FUNC;
}



void SymbolImpl::set_deferred_breakpoint(
    BreakPoint::Type    type,
    Runnable*           runnable,
    BreakPoint::Action* action,
    Symbol*             sym)
{
    if (sym == NULL)
    {
        sym = this;
    }
    if (RefPtr<SymbolTableBase> table = interface_cast<SymbolTableBase>(table_.ref_ptr()))
    {
        if (table->addr())
        {
            throw logic_error(__func__ + string(": table already mapped"));
        }
        table->set_deferred_breakpoint(*sym, type, runnable, action);
    }
}


void SymbolImpl::trim_memory()
{
    if (demangledName_)
    {
        if (demangledName_->ref_count() <= 1)
        {
            demangledName_.reset();
            assert(!demangledName_);
        }
    }
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
