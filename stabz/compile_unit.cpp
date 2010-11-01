//
// $Id: compile_unit.cpp 720 2010-10-28 06:37:54Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <cassert>
#include <stdexcept>
#include <memory>
#include "dharma/config.h"
#include "dharma/environ.h"
#include "generic/nearest.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "public/compile_unit.h"
#include "public/function.h"
#include "public/variable.h"
#include "private/parse_events.h"
#include "private/throw.h"
#include "private/util.h"

using namespace std;


static RefPtr<SharedString> empty_string()
{
    static RefPtr<SharedString> empty(shared_string(""));
    return empty;
}


Stab::CompileUnit::CompileUnit
(
    Descriptor&     desc,
    SharedString&   section,
    SharedString*   buildPath,
    const char*     fileName,
    addr_t          beginAddr,
    size_t          beginIndex
)
: desc_(&desc)
, section_(&section)
, buildPath_(buildPath)
, beginAddr_(beginAddr)
, endAddr_(0)
, beginIndex_(beginIndex)
, endIndex_(0)
, parsed_(false)
{
    methodMap_.set_empty_key(RefPtr<SharedString>());
    if (buildPath_.is_null())
    {
        buildPath_ = empty_string();
    }
    assert(fileName);
    shortName_ = shared_string(fileName);
    name_ = SharedStringImpl::take_ownership(fullpath(buildPath_->c_str(), fileName));

    typeTables_.reserve(16);

    // always start with one type table
    typeTables_.push_back(TypeTablePtr(new TypeTable));
}


Stab::CompileUnit::~CompileUnit() throw()
{
}


SharedString& Stab::CompileUnit::section() const
{
    assert(!section_.is_null());
    return *section_;
}


void Stab::CompileUnit::set_end_index(size_t index)
{
    assert(endIndex_ == 0);
    assert(index >= beginIndex_);

    endIndex_ = index;
}


void Stab::CompileUnit::set_end_addr(addr_t addr)
{
    assert(endAddr_ == 0);
    assert(addr >= beginAddr_);

    endAddr_ = addr;
}


size_t Stab::CompileUnit::size() const
{
    assert(beginAddr_);

    return endAddr_? endAddr_ - beginAddr_ : 0;
}


SharedString& Stab::CompileUnit::build_path() const
{
    assert(!buildPath_.is_null());
    return *buildPath_;
}


/*
SharedString& Stab::CompileUnit::name() const
{
    assert(!name_.is_null());
    return *name_.get();
}
*/


void
Stab::CompileUnit::add_function(const RefPtr<Stab::Function>& func)
{
    FunctionMap::iterator i = functionMap_.find(func->begin_addr());
    if (i == functionMap_.end())
    {
        functionMap_.insert(i, make_pair(func->begin_addr(), func));
    }
    else
    {
#if DEBUG && !PROFILE
        if (i->second.get() != func.get())
        {
            string msg = "Overriding ";
            msg += i->second->name().c_str();
            msg += " with: ";
            msg += func->name().c_str();

            throw runtime_error(msg);
        }
        assert(i->second.get() == func.get());
#endif
        i->second = func;
    }
}


Stab::Function*
Stab::CompileUnit::lookup_function(addr_t addr, bool strict) const
{
    if (strict)
    {
        FunctionMap::const_iterator i = functionMap_.find(addr);
        if (i == functionMap_.end())
        {
            return NULL;
        }
        return i->second.get();
    }

    FunctionMap::const_iterator i = functionMap_.lower_bound(addr);
    if ((i == functionMap_.end()) || (i->first != addr))
    {
        if (i == functionMap_.begin())
        {
            return NULL;
        }
        --i;
    }
    return i->second.get();
}


vector<RefPtr<Stab::Function> > Stab::CompileUnit::functions() const
{
    FunList funcs;
    funcs.reserve(functionMap_.size());

    FunctionMap::const_iterator i = functionMap_.begin();
    for (; i != functionMap_.end(); ++i)
    {
        funcs.push_back(i->second);
    }

    return funcs;
}


/*
void Stab::CompileUnit::add_source_line(
    const RefPtr<SharedString>& sourceFile,
    size_t lineNum,
    addr_t addr)
{
    if (addr)
    {
        assert(!sourceFile.is_null());
        lineByAddr_.insert(make_pair(addr, make_pair(sourceFile, lineNum)));
    }
}
*/


static bool use_nearest_match()
{
    static bool flag = env::get_bool("ZERO_LINE_NEAREST_MATCH", true);
    return flag;
}


size_t Stab::CompileUnit::addr_to_line(
    addr_t  addr,
    addr_t* nearest,
    EnumCallback2<SharedString*, size_t>* events) const
{
    size_t count = 0;

/* if nearest match not specified, do an exact lookup */
    if (!nearest)
    {
        LineByAddr::const_iterator i = lineByAddr_.find(addr);
        for (; i != lineByAddr_.end() && i->first == addr; ++i)
        {
            if (events)
            {
                events->notify(i->second.first.get(), i->second.second);
            }
            ++count;
        }
        return count;
    }

/* ... otherwise find the nearest line(s) for the given addr */
    assert(nearest);

    LineByAddr::const_iterator i = lineByAddr_.lower_bound(addr);

    if (i == lineByAddr_.end() || i->first != addr)
    {
        if (i == lineByAddr_.begin())
        {
            return 0;
        }
        LineByAddr::const_iterator j = i;
        --j;
        if (!use_nearest_match() || compare_nearest(addr, j->first, i->first))
        {
            --i;
        }
        const addr_t nearestAddr = i->first;

        if (compare_nearest(addr, *nearest, nearestAddr))
        {
            return 0; // nearestAddr is not the nearest match
        }

        *nearest = nearestAddr;
        while (i->first == nearestAddr && i != lineByAddr_.begin())
        {
            --i;
        }
        if (i->first != nearestAddr)
        {
            ++i;
        }
        assert(i->first == nearestAddr);
    }
    else
    {
        assert(i->first == addr);
        *nearest = i->first;
    }

    for (addr = i->first;
         i->first == addr && i != lineByAddr_.end();
         ++i, ++count)
    {
        if (events)
        {
            events->notify(i->second.first.get(), i->second.second);

            if (i->second.first->is_equal(name().c_str()))
            {
                break;
            }
        }
    }
    return count;
}


addr_t Stab::CompileUnit::next_line(
    RefPtr<SharedString>& sourceFile,
    size_t lineNum,
    addr_t addr,
    size_t* nextLineNum) const
{
    LineByAddr::const_iterator i = lineByAddr_.upper_bound(addr);
    if (i == lineByAddr_.end())
    {
        addr = 0;
    }
    else
    {
        sourceFile = i->second.first;
        if (nextLineNum)
        {
            /* todo: if several entries are found in the
               multimap, which one should I pick? */
            *nextLineNum = i->second.second;
        }
        addr = i->first;
    }
    return addr;
}


/* todo: this may be somewhat misleading; we're not limiting the lookup
   to this very unit; rather, we're delegating to the parent descriptor. */
RefPtr<DataType> Stab::CompileUnit::lookup_type(SharedString& name) const
{
    RefPtr<DataType> type;
    Descriptor::TypeMap& typeMap = CHKPTR(desc_)->type_map();

    Descriptor::TypeMap::const_iterator i = typeMap.find(&name);
    if (i != typeMap.end())
    {
        type = i->second.ref_ptr();
    }
    return type;
}


void Stab::CompileUnit::add_method
(
    SharedString& name,
    const RefPtr<MethodImpl>& method
)
{
    MethodMap::iterator i = methodMap_.find(&name);

    if (i == methodMap_.end())
    {
        //methodMap_.insert(i, make_pair(&name, method));
        methodMap_.insert(make_pair(&name, method));
    }
}



RefPtr<MethodImpl> Stab::CompileUnit::lookup_method(SharedString& name) const
{
    RefPtr<MethodImpl> method;
    MethodMap::const_iterator i = methodMap_.find(&name);

    if (i != methodMap_.end())
    {
        method = i->second;
    }
    return method;
}



size_t Stab::CompileUnit::enum_sources(EnumCallback<SharedString*, bool>* callback)
{
    size_t count = 0;

    SourceSet::const_iterator i = sources_.begin();
    for (; i != sources_.end(); ++i, ++count)
    {
        if (callback)
        {
            if (!callback->notify(i->get()))
            {
                break;
            }
        }
    }
    return count;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
