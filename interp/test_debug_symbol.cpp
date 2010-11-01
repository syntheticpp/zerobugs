// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: test_debug_symbol.cpp 714 2010-10-17 10:03:52Z root $
//
#include <stdexcept>
#include <string>
#include <sstream>
#include "zdk/data_type.h"
#include "zdk/shared_string_impl.h"
#include "test_debug_symbol.h"


TestDebugSymbol::TestDebugSymbol(DataType& type, const char* name, const char* value)
    : type_(&type)
    , addr_(0x40001234)
    , constant_(false)
{
    assert(name);
    assert(value);

    name_ = shared_string(name);
    value_ = shared_string(value);
}


TestDebugSymbol::TestDebugSymbol(const TestDebugSymbol& other)
    : type_(other.type_)
    , addr_(other.addr_)
    , name_(other.name_)
    , value_(other.value_)
    , constant_(other.constant_)
    , children_(other.children_)
{
}


SharedString* TestDebugSymbol::name() const
{
    return name_.get();
}


SharedString* TestDebugSymbol::value() const
{
    return value_.get();
}


DataType* TestDebugSymbol::type() const
{
    return type_.get();
}


int TestDebugSymbol::compare(const DebugSymbol* other) const
{
    assert(other);
    return type()->compare(value()->c_str(), other->value()->c_str());
}


DebugSymbol* TestDebugSymbol::child(size_t n) const
{
    if (n >= children_.size())
    {
        std::ostringstream err;

        err << __func__ << ": " << name() << " has ";
        err << children_.size() << " children, ";
        err << n << " is out of range.";

        throw std::out_of_range(err.str());
    }
    return children_[n].get();
}


void TestDebugSymbol::add_child(DebugSymbol* sym)
{
    if (sym)
    {
        children_.push_back(sym);
        // todo: set depth, parent
    }
    assert(children_.size());
}


size_t TestDebugSymbol::enum_children(DebugSymbolCallback* cb) const
{
    if (cb)
    {
        Children::const_iterator i = children_.begin();
        for (; i != children_.end(); ++i)
        {
            cb->notify(i->get());
        }
    }
    return children_.size();
}


void TestDebugSymbol::read(DebugSymbolEvents*, long)
{
    //throw std::logic_error(std::string("not implemented: ") + __func__);
}


size_t TestDebugSymbol::write(const char* value)
{
    assert(value);
    value_ = shared_string(value);

    return value_->length();
}



DebugSymbol* TestDebugSymbol::clone
(
    SharedString*   value,
    DataType*       type,
    bool            isConst
) const
{
    TestDebugSymbol* dsym = new TestDebugSymbol(*this);

    if (type)
    {
        dsym->type_ = type;
    }
    if (value)
    {
        dsym->addr_ = 0;

        // clog << __func__ << endl;

        dsym->value_ = value;
        dsym->constant_ = true;
    }
    return dsym;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
