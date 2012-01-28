//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
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
#include <stdexcept>
#include "public/complex.h"
#include "public/debug_symbol.h"
#include "zdk/check_ptr.h"

using namespace std;


ComplexImpl::ComplexImpl(SharedString* name, DataType& type)
    : BaseType(name, 2 * type.bit_size())
    , partType_(&type)
{
}


int ComplexImpl::compare(const char*, const char*) const
{
    throw runtime_error("Complex::compare not implemented");
}


SharedString*
ComplexImpl::read(DebugSymbol* sym, DebugSymbolEvents* events) const
{
    const addr_t addr = CHKPTR(sym)->addr();
    DebugSymbolImpl* impl = interface_cast<DebugSymbolImpl*>(sym);
    if (!impl)
    {
        throw runtime_error("unknown implementation");
    }

    try
    {
        if (partType_)
        {
            RefPtr<SharedString> real = partType_->read(sym, events);

            impl->set_addr(addr + partType_->size());
            RefPtr<SharedString> imag = partType_->read(sym, events);

            impl->set_addr(addr);

            real = real->append(" + ");
            real = real->append(imag->c_str());
            real = real->append("i");

            return real.detach();
        }
    }
    catch (...)
    {
        impl->set_addr(addr);
        throw;
    }
    return NULL;
}


void ComplexImpl::write(DebugSymbol* sym, const Buffer*) const
{
    throw runtime_error("Complex::write not implemented");
}


bool ComplexImpl::is_equal(const DataType* type) const
{
    if (const ComplexImpl* other = interface_cast<const ComplexImpl*>(type))
    {
        return partType_->is_equal(other->partType_.get());
    }
    return false;
}


size_t ComplexImpl::parse(const char*, Unknown2*) const
{
    //
    // todo
    //
    return 0;
}
