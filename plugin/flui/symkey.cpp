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
#include "symkey.h"
#include "zdk/debug_sym.h"

using namespace std;


SymKey::SymKey(DebugSymbol& sym)
    : addr_(sym.addr())
    , depth_(sym.depth())
    , hash_(0)
    , name_(sym.name())
{
    if (SharedString* name = sym.name())
    {
        hash_ = name->hash();
    }
}


#define RET_IF_NOT_EQUAL(lhs, rhs, field) \
    if ((lhs).field < (rhs).field) return true; \
    if ((lhs).field > (rhs).field) return false;

bool operator<(const SymKey& lhs, const SymKey& rhs)
{
    RET_IF_NOT_EQUAL(lhs, rhs, depth_);

    RET_IF_NOT_EQUAL(lhs, rhs, addr_);
    RET_IF_NOT_EQUAL(lhs, rhs, hash_);

    RefPtr<SharedString> lptr = lhs.name_.ref_ptr();
    RefPtr<SharedString> rptr = rhs.name_.ref_ptr();

    return lptr && rptr && lptr->is_less(rptr->c_str());
}


#undef RET_IF_NOT_EQUAL
#define RET_IF_NOT_EQUAL(lhs, rhs, field) \
    if ((lhs).field != (rhs).field) return false; \

bool operator==(const SymKey& lhs, const SymKey& rhs)
{
    RET_IF_NOT_EQUAL(lhs, rhs, depth_);

    RET_IF_NOT_EQUAL(lhs, rhs, addr_);
    RET_IF_NOT_EQUAL(lhs, rhs, hash_);

    RefPtr<SharedString> lptr = lhs.name_.ref_ptr();
    RefPtr<SharedString> rptr = rhs.name_.ref_ptr();

    return (!lptr && !rptr) || lptr->is_equal(rptr->c_str());
}

void SymKey::print(ostream& out) const
{
    out << "(" <<(void*)addr_ << "/" << depth_ << "/#" << hash_ << ")";
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

