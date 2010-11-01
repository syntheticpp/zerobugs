//
// $Id: debug_symbol_vector.cpp 729 2010-10-31 07:00:15Z root $
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
#include <sstream>
#include "public/debug_symbol.h"
#include "public/debug_symbol_vector.h"

using namespace std;


DebugSymbolVector::DebugSymbolVector()
{
}


DebugSymbolVector::DebugSymbolVector(const DebugSymbolVector& other)
    : method_(other.method_)
{
    v_.reserve(other.v_.size());

    // deep copy the elements:
    Vector::const_iterator i = other.v_.begin();
    for (; i != other.v_.end(); ++i)
    {
        v_.push_back(new DebugSymbolImpl(**i));
    }
}


DebugSymbolVector::~DebugSymbolVector() throw()
{
    try
    {
        detach_from_parent();
    }
    catch (...)
    {
    }
}


void
DebugSymbolVector::add(const RefPtr<DebugSymbolImpl>& sym)
{
    assert(sym);

    if (!v_.empty())
    {
        // filter adjacent duplicates; duplicates may be the
        // result of a variable been referenced by a CPU reg

        RefPtr<DebugSymbol> prev = v_.back();
        if (prev->addr() == sym->addr()
         && prev->name()->is_equal2(sym->name()))
        {
            if (prev->value() && prev->value()->length())
            {
                return;
            }
            else
            {
                v_.pop_back();
            }
        }
    }

    v_.push_back(sym);
}



size_t
DebugSymbolVector::enumerate(DebugSymbolCallback* callback) const
{
    if (callback)
    {
        for (Vector::const_iterator i = v_.begin(); i != v_.end(); ++i)
        {
            callback->notify(i->get());
        }
    }
    return v_.size();
}



void
DebugSymbolVector::set_method(Method* method)
{
    assert(!method_.ref_ptr());

    method_ = method;

    assert(method_.ref_ptr().is_null() || v_.empty());
}


RefPtr<Method> DebugSymbolVector::method() const
{
    RefPtr<Method> m = method_.ref_ptr();

    return m;
}



void
DebugSymbolVector::detach_from_parent()
{
    for (Vector::const_iterator i = v_.begin(); i != v_.end(); ++i)
    {
        (*i)->detach_from_parent();
    }
}


DebugSymbol*
DebugSymbolVector::nth_child(DebugSymbol& sym, size_t n)
{
    if (n >= v_.size())
    {
        ostringstream err;

        const char* name = sym.name() ? sym.name()->c_str() : "?";
        err << __func__ << ": " << name << " has ";
        err << v_.size() << " children, ";
        err << n << " is out of range.";

        throw out_of_range(err.str());
    }
    return v_[n].get();
}


DebugSymbolCollection* DebugSymbolVector::clone() const
{
    return new DebugSymbolVector(*this);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
