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

#include "zdk/align.h"
#include "zdk/check_ptr.h"
#include "zdk/shared_string_impl.h"
#include "zdk/variant.h"
#include "zdk/zero.h"
#include "zdk/32_on_64.h"
#include "debug_out.h"
#include "dharma/variant_impl.h"
#include "frame_setup.h"
#include "target/target.h"
#include "typez/public/debug_symbol.h"
#include <vector>

using namespace std;


static void
copy_onto_stack(Thread& thread, reg_t& stackPtr, const void* data, size_t nbytes)
{
    assert(stackPtr);
    assert(data);
    assert(nbytes);

    size_t nwords = round_to_word(nbytes);
    assert(nwords);

    vector<word_t> buf(nwords);

    stackPtr -= nwords * sizeof(word_t);

    //
    // debugging 32-bit process on 64-bit host?
    //
    if ((__WORDSIZE > 32) && thread.is_32_bit())
    {
        DEBUG_OUT << "arg size=" << nbytes  << ", "
                  << (round_to<int32_t>(nbytes) % 2) << "\n";

        if (round_to<int32_t>(nbytes) % 2)
        {
            stackPtr += sizeof(int32_t);

            const size_t back = nwords - 1;
            thread.read_data(stackPtr + back, &buf[back], 1);
        }
    }

    memcpy(&buf[0], data, nbytes);
    thread.write_data(stackPtr, &buf[0], nwords);
}


addr_t
FrameSetup::push_arg(Thread& thread, const Variant& v, addr_t sp)
{
    assert(sp);
    if (!stackPtr_)
    {
        assert(thread.stack_pointer() >= sp);
        stackPtr_ = sp;
    }

    assert(sp <= stackPtr_);

    set_red_zone(thread, sp);

    if (size_t size = v.size())
    {
        if (v.type_tag() == Variant::VT_POINTER)
        {
            if (thread.is_32_bit())
            {
                uint32_t addr = v.pointer();
                copy_onto_stack(thread, sp, &addr, sizeof(addr));
            }
            else
            {
                addr_t addr = v.pointer();
                copy_onto_stack(thread, sp, &addr, sizeof(addr));
            }
        }
        else
        {
            copy_onto_stack(thread, sp, v.data(), size);
        }
    }
    else if (DebugSymbol* sym = v.debug_symbol())
    {
        DataType* type = CHKPTR(sym->type());

        size_t size = type->size();
        if (thread.is_32_bit() &&
            (v.type_tag() == Variant::VT_POINTER || interface_cast<PointerType*>(type)))
        {
            size = 4;
        }
        const size_t nwords = round_to_word(size);
        vector<word_t> buf(nwords);

        thread.read_data(sym->addr(), &buf[0], nwords);
        copy_onto_stack(thread, sp, &buf[0], size);
    }

    return stackPtr_ = sp;
}



RefPtr<Variant>
FrameSetup::push_literal(Thread& thread, DebugSymbol* sym, addr_t& sp)
{
    RefPtr<Variant> result;

    if (sym)
    {
        set_red_zone(thread, sp);

        // string literals typed in by the user do not
        // necessarily exist in the debugged program's
        // memory space: create them on the stack

        const size_t len = sym->value()->length() + 1;
        const char* str = sym->value()->c_str();

        copy_onto_stack(thread, sp, str, len);

        ostringstream addr;
        addr << sp;
        RefPtr<SharedString> val = shared_string(addr.str());

        RefPtr<DebugSymbol> tmp = sym->clone(val.get());
        interface_cast<DebugSymbolImpl&>(*tmp).set_addr(sp);
        result.reset(new VariantImpl(*tmp));

        stackPtr_ = sp;
    }

    return result;
}



void
FrameSetup::set_red_zone(Thread& thread, addr_t& sp)
{
    assert(sp);

    if (!redZoneSet_)
    {
        redZoneSet_ = true;

        if (RefPtr<Target> target = thread.target())
        {
            stackPtr_ = sp = target->stack_reserve(thread, sp);
        }
    }
}



addr_t
FrameSetup::reserve_stack(Thread& thread, size_t size)
{
    if (!stackPtr_)
    {
        stackPtr_ = thread.stack_pointer();
    }
    stackPtr_ -= size;
    return stackPtr_;
}


// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
