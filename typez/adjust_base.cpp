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

#include <iostream>
#include <stdexcept>
#include "dharma/environ.h"
#include "generic/state_saver.h"
#include "zdk/check_ptr.h"
#include "zdk/thread_util.h"
#include "zdk/32_on_64.h"
#include "public/adjust_base.h"
#include "public/types.h"

#ifdef DEBUG
 #include <iostream>
#endif

using namespace std;


////////////////////////////////////////////////////////////////
static bool debug_vtables()
{
    static bool flag = env::get_bool("ZERO_DEBUG_VTABLES");
    return flag;
}


////////////////////////////////////////////////////////////////
// Read vptr from given address (in the context of the given thread),
// then read an adjustment value from the vtable (the vcall offset).
//
off_t
get_vtable_adjustment(Thread& thread, addr_t addr, off_t offs)
{
    // find the address of the vtable (assume that vptr is the
    // first data member in the object).
    addr_t vptr = 0;
    thread_read(thread, addr, vptr);

    if (debug_vtables())
    {
        StateSaver<ios, ios::fmtflags>__(clog);
        clog << __func__ << hex << ": addr=" << addr;
        clog << ", vptr=" << vptr << endl;
    }

    long adj = 0;

    if (vptr)
    {
        thread_read(thread, vptr + offs, adj);

        if (debug_vtables())
        {
            StateSaver<ios, ios::fmtflags>__(clog);
            clog << __func__ << ": " << adj << " (0x" << hex << adj << ")\n";
        }
    }
    return adj;
}



////////////////////////////////////////////////////////////////
off_t
ClassTypeImpl::offset_to_top (
    Thread&             thread,
    const BaseClass&    base,
    addr_t              addr,
    off_t               offs)
{
    ClassTypeImpl* klass = 0;

    if (base.type()->query_interface(_uuid(), (void**)&klass))
    {
        assert(klass);

        if (RefPtr<RTTI> rtti = klass->rtti(&thread))
        {
            addr += rtti->vptr_offset();

            offs = 0;

            //http://www.codesourcery.com/cxx-abi/abi.html#vtable
            //typeinfo ptr is at index -1 from vtable addr
            //(where vptr points) and is always present;
            //offset-to-top is at index -2 (i.e. -2 * sizeof(void*))
        #ifdef DEBUG
            word_t toTop = addr;
            Platform::dec_word_ptr(thread, toTop, 2);
            thread_read(thread,  toTop, toTop);
        #endif
            // decrement pointer, using the pointer size
            // for the target platform
            Platform::dec_word_ptr(thread, offs, 3);

            offs = get_vtable_adjustment(thread, addr, offs);
        #ifdef DEBUG
            clog << __func__ << ": toTop=" << toTop << " offs=" << offs << endl;
        #endif
            return offs;
        }
    }
    // todo: compute the offset according to the CXX ABI
    // rather than throwing an exception
    string err = CHKPTR(CHKPTR(base.type())->name())->c_str();
    err += ": no vtable, cannot cast to derived";
    throw runtime_error(err);

    return offs;
}


////////////////////////////////////////////////////////////////
addr_t
adjust_base_to_derived (
    Thread&             thread,     // thread of context
    addr_t              addr,       // address of instance of KLASS
    const char*         debugFormat,
    const ClassType&    klass,      // possible base of DERIVED
    const ClassType&    derived)
{
    const SharedString* baseName = CHKPTR(klass.name());

    // first, check if KLASS and DERIVED are of one and the same type
    if (derived.is_equal(&klass))
    {
    #ifdef DEBUG
        clog << __func__ << ": identity: " << baseName << endl;
    #endif
        return 0; // no adjustment done
    }

    off_t offset = 0;

    // is KLASS a base of DERIVED'?
    const BaseClass* base = derived.lookup_base(baseName, &offset, true);

    if (!base) // nope
    {
        return 0;
    }
    if ((__WORDSIZE > 32) && thread.is_32_bit()
        && (offset & 0xffffffff00000000LL) == 0)
    {
        int32_t tmp = offset;
        offset = tmp;
    }

    if (!base->virtual_index()) // is it a virtual base?
    {
        assert(offset >= 0);
        addr -= offset;
    }
    else if (strcmp(debugFormat, "stabs") == 0)
    {
        if (offset > 0)
        {
            addr -= offset;
        }
        else
        {
            offset = ClassTypeImpl::offset_to_top(thread, *base, addr, offset);

            addr += offset;
        }
    }
    else
    {
        // assume that the debug info tells us how to
        // find the top object -- as in DWARF
        if (offset > 0) // fixme: possible problem during construction
        {
            offset = -offset;
        }

        addr += offset;
    }

    return addr;
}


addr_t
adjust_base_to_derived (
    const DebugSymbol&  dsym,
    const ClassType&    klass,
    const ClassType&    derived)
{
    const char* format = dsym.reader() ? dsym.reader()->format() : "unknown";
    return adjust_base_to_derived(*CHKPTR(dsym.thread()),
                                  dsym.addr(),
                                  format,
                                  klass,
                                  derived);
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
