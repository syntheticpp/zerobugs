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
#include <assert.h>
#include "zdk/shared_string_impl.h"
#include "private/link_data.h"


LinkDataImpl::LinkDataImpl(const RefPtr<SymbolTable>& table)
    : table_(table)
    , addr_(table->addr())
    , adjust_(table->adjustment())
    , name_(table->filename())
{
}


LinkDataImpl::LinkDataImpl(addr_t addr, const char* path)
    : addr_(addr)
    , adjust_(0)
    , name_(SharedStringImpl::create(path))
{
}


LinkDataImpl::~LinkDataImpl() throw()
{
}


void LinkDataImpl::set_next(RefPtr<LinkDataImpl> next)
{
    assert(!next_);
    next_ = next;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
