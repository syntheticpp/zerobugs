//
// $Id: dynamic_array.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sstream>
#include <boost/limits.hpp>
#include "dynamic_array.h"
#include "typez/public/debug_symbol_array.h"
#include "typez/public/is_cv_qualified.h"
#include "typez/public/util.h"
#include "zdk/32_on_64.h"
#include "zdk/thread_util.h"
#include "zdk/type_system.h"
#include "parse_array.h"

using namespace std;


/**
 * Derive the name of the array type from the elements' type
 */
static RefPtr<SharedString>
make_name(TypeSystem& typesys, DataType& elemType)
{
    ostringstream buf;

    buf << elemType.name()->c_str() << "[]";
    return typesys.get_string(buf.str().c_str(), buf.str().length());
}


DynamicArray::DynamicArray(TypeSystem& typesys, DataType& elemType)
    : BaseType(make_name(typesys, elemType), typesys.word_size() * 2)
    , elemType_(&elemType)
    , wordSize_(typesys.word_size())
{
    typesys.manage(this);
}


bool DynamicArray::is_equal(const DataType* type) const
{
    if (const DynamicArray* other =
            interface_cast<const DynamicArray*>(type))
    {
        return elem_type()->is_equal(other->elem_type())
            && !is_cv_qualified(type);
    }
    return false;
}


SharedString*
DynamicArray::read(DebugSymbol* sym, DebugSymbolEvents* events) const
{
    assert(sym);

    RefPtr<DataType> elemType(elemType_.ref_ptr());
    if (!elemType)
    {
        return SharedStringImpl::create("#error: null elem type");
    }

    addr_t addr = sym->addr();

    RefPtr<Thread> thread = sym->thread();

    if (!addr || !thread)
    {
        return null_ptr();
    }
    DebugSymbolImpl* array = interface_cast<DebugSymbolImpl*>(sym);
    if (!array)
    {
        return SharedStringImpl::create("#error: unknown array implementation");
    }

    if (size_t nelem = count(array))
    {
        addr = first_elem_addr(sym);

        DebugSymbolArray* elems = dynamic_cast<DebugSymbolArray*>(array->children().get());

        if (!elems || (elems->size() != nelem))
        {
            elems = new DebugSymbolArray;
            array->set_children(auto_ptr<DebugSymbolCollection>(elems));
        }
        --nelem; // the call below expects the index of the last elem
        //
        // delegate the reading of array elements
        //
        elems->read(sym->reader(), *array, addr, nelem, *elemType, events);
    }
    ostringstream buf;
    buf << '[' << hex << "0x" << addr << ']';

    return shared_string(buf.str()).detach();
}



/**
 * @note the correct thing is for caller code to call count()
 */
size_t DynamicArray::elem_count() const
{
    return numeric_limits<size_t>::max();
}


/**
 * Assume D language ABI: a packed struct with size first,
 * and pointer to elems second
 */
addr_t
DynamicArray::first_elem_addr(DebugSymbol* sym) const
{
    if (sym)
    {
        if (RefPtr<Thread> thread = sym->thread())
        {
            addr_t addr = sym->addr();

            if (thread->is_32_bit())
            {
                addr += 4;
            }
            else
            {
                addr += sizeof(word_t);
            }
            thread_read(*thread, addr, addr);
            return addr;
        }
    }
    return 0;
}



/**
 * @return the number of elements currently in a
 * dynamic array
 *
 * Assume D language ABI: a packed struct with size first,
 * and pointer to elems second
 */
size_t DynamicArray::count(DebugSymbol* sym) const
{
    size_t nelem = 0;

    if (sym)
    {
        if (RefPtr<Thread> thread = sym->thread())
        {
            thread_read(*thread, sym->addr(), nelem);
            Platform::after_read(*thread, nelem);
        }
    }
    return nelem;
}



size_t DynamicArray::parse(const char* str, Unknown2* unk) const
{
    return parse_array(str, unk, wordSize_);
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
