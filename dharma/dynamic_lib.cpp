//
// $Id$
//
// Platform-independent part of the DynamicLib class implementation.
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iostream>
#include <stdexcept>
#include "canonical_path.h"
#include "dynamic_lib.h"
#include "process_name.h"

using namespace std;


////////////////////////////////////////////////////////////////
bad_downcast::bad_downcast(
    const type_info& fromType,
    const type_info& toType
    )
{
    what_ = "cannot downcast ";
    what_ += fromType.name();
    what_ += " to ";
    what_ += toType.name();
}


////////////////////////////////////////////////////////////////
DynamicLib::DynamicLib(const char* fileName)
    : handle_(0)
    , isSelf_(fileName == 0)
{
    if (fileName)
    {
        filename_ = canonical_path(fileName);
    }
    else
    {
        filename_ = realpath_process_name();
    }

    if (!isSelf_)
    {
        if (filename_ == realpath_process_name())
        {
            isSelf_ = true;
        }
    }
}

////////////////////////////////////////////////////////////////
DynamicLib::~DynamicLib() throw()
{
}

////////////////////////////////////////////////////////////////
void DynamicLib::unload() throw()
{
    if (handle_.get())
    {
        clog << "Unloading: " << filename() << endl;
        handle_.reset(0);
    }
}

////////////////////////////////////////////////////////////////
DynamicLib::Counter::Counter(int n, DynamicLib* module)
    : count_(0)
{
    if (module)
    {
        handle_ = module->handle();
        assert(handle_.count() >= 2);
    }
    while (count_ != n)
    {
        ++*this;
    }
}

////////////////////////////////////////////////////////////////
DynamicLib::Counter::Counter(const Counter& other)
    : count_(other.count_)
    , handle_(other.handle_)
{
#ifdef DEBUG
    print(clog, __func__);
#endif
}


////////////////////////////////////////////////////////////////
ostream&
DynamicLib::Counter::print(ostream& outs, const char* func) const
{
    outs << func << ": " << handle_.get();
    outs << " count=" << count_ << endl;

    return outs;
}


////////////////////////////////////////////////////////////////
DynamicLib::Counter::~Counter() throw()
{
}

////////////////////////////////////////////////////////////////
DynamicLib::Counter& DynamicLib::Counter::operator++()
{
    ++count_;

    return *this;
}

////////////////////////////////////////////////////////////////
DynamicLib::Counter& DynamicLib::Counter::operator--()
{
    assert(count_ > 0);
    --count_;

    return *this;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

