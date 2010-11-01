//
// $Id: debug_symbol_array.cpp 720 2010-10-28 06:37:54Z root $
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
#include "dharma/environ.h"
#include "generic/temporary.h"
#include "zdk/interface_cast.h"
#include "typez/public/debug_symbol.h"
#include "typez/public/debug_symbol_array.h"


using namespace std;
using namespace Platform;


static ArrayType& get_array_type(DebugSymbol& sym)
{
    ArrayType* arrayType = interface_cast<ArrayType*>(sym.type());

    if (!arrayType)
    {
        ostringstream err;
        err << __func__ << ": " << sym.name() << ": type is not array";
        throw logic_error(err.str());
    }
    return *arrayType;
}


size_t max_array_range()
{
    static const size_t maxRange = env::get("ZERO_MAX_ARRAY", 100);
    return maxRange;
}



DebugSymbolCollection* DebugSymbolArray::clone() const
{
    return new DebugSymbolArray(*this);
}


static RefPtr<SharedString>
elem_name(const DebugSymbol& array, addr_t addr, size_t i)
{
    ostringstream buf;

    // print the index (assuming the array starts at 0)
    //  this works fine for C and C++
    buf << array.name() << '[' << i << ']';
    //buf << " (0x" << hex << addr << ")";

    return shared_string(buf.str());
}


RefPtr<DebugSymbolImpl>
DebugSymbolArray::add_element (
    DebugInfoReader* reader,
    Thread& thread,
    DebugSymbolImpl& array,
    DataType& elemType,
    DebugSymbolEvents* events,
    addr_t addr,
    uint64_t index,
    uint64_t offset)
{
    const addr_t a = addr + elemType.size() * index;

    RefPtr<SharedString> name = elem_name(array, a, index + offset);

    RefPtr<DebugSymbolImpl> child =
        DebugSymbolImpl::create(reader, thread, elemType, *name, a);

    array.add_child_impl(child.get());

    if (events && events->is_expanding(&array))
    {
        child->read(events);
    }
    return child;
}



void DebugSymbolArray::read(DebugInfoReader* reader,
                            DebugSymbolImpl& array,
                            const addr_t addr,
                            uint64_t lower,
                            uint64_t upper,
                            DataType& elemType,
                            DebugSymbolEvents* events,
                            uint64_t offset)
{
    assert(empty()); // precondition

    RefPtr<Thread> thread = array.thread();

    upper = min(upper, lower + max_array_range());

    ArrayType& arrayType = get_array_type(array);
    if (arrayType.elem_count() == 0)
    {
        return;
    }

    uint64_t upperElem = arrayType.elem_count() - 1;

    if (DynamicArrayType* dynamicArray =
            interface_cast<DynamicArrayType*>(&arrayType))
    {
        upperElem = dynamicArray->count(&array);

        if (upperElem)
        {
            --upperElem;
        }
    }
    else
    {
        upper = min(upper, upperElem);
    }

    for (uint64_t i = lower; i <= upper; ++i)
    {
        RefPtr<DebugSymbolImpl> child;

        child = add_element(reader,
                            *thread,
                            array,
                            elemType,
                            events,
                            addr,
                            i, offset);
    }
    if (upper < upperElem)
    {
        RefPtr<DebugSymbolImpl> child =
               DebugSymbolImpl::create(*thread, elemType,
                                        "",
                                        shared_string("..."));

        array.add_child_impl(child.get());

        if (upperElem > 1 && upper < upperElem - 1)
        {
            add_element(reader, *thread, array, elemType,
                        events, addr, upperElem - 1, offset);
        }
        add_element(reader, *thread, array, elemType,
                    events, addr, upperElem, offset);

    }
}



void DebugSymbolArray::read(DebugInfoReader* reader,
                            DebugSymbolImpl& array,
                            addr_t addr,
                            uint64_t upper,
                            DataType& elemType,
                            DebugSymbolEvents* events,
                            uint64_t offset)
{
    read(reader, array, addr, 0, upper, elemType, events, offset);
}



DebugSymbol* DebugSymbolArray::nth_child(DebugSymbol& sym, size_t n)
{
    if ((n >= size()) || (n > max_array_range()))
    {
        ArrayType& arrayType = get_array_type(sym);

        if (n < arrayType.elem_count())
        {
            DataType* elemType = arrayType.elem_type();

            if (!elemType)
            {
                ostringstream err;
                err << __func__ << ": array has null element type";

                throw logic_error(err.str());
            }
            DebugInfoReader* reader = sym.reader();
            RefPtr<Thread> thread = sym.thread();

            addr_t addr = sym.addr() + n * elemType->size();

            RefPtr<DebugSymbolImpl> child =
                DebugSymbolImpl::create(reader,
                                        *thread,
                                        *elemType,
                                        *elem_name(sym, addr, n),
                                        addr);

            Temporary<bool> setFlag(sparse_, true);
            sym.add_child(child.get());

            child->read(NULL);
            return child.detach();
        }
    }
    return DebugSymbolVector::nth_child(sym, n);
}



void DebugSymbolArray::add(const RefPtr<DebugSymbolImpl>& sym)
{
    if (!sparse_)
    {
        DebugSymbolVector::add(sym);
    }
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
