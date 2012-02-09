// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id$
//
#include "zdk/log.h"
#include "zdk/thread_util.h"
#include "dharma/environ.h"
#include "public/types.h"
#include "public/util.h"
#include "private/debug_rtti.h"
#include "unmangle/unmangle.h"

using namespace std;
using Platform::byte_size;




////////////////////////////////////////////////////////////////
static bool always_scan_vtables()
{
    static bool flag = env::get_bool("ZERO_ALWAYS_SCAN_VTABLES");
    return flag;
}


////////////////////////////////////////////////////////////////
RTTI::RTTI(const RefPtr<MemberImpl>& vptr)
    : processing_(false)
    , bitOffset_(0)
    , offset_(0)
{
    assert(vptr);

    bitOffset_ = vptr->bit_offset();
    assert((bitOffset_ % Platform::byte_size) == 0);
    offset_ = bitOffset_ / Platform::byte_size;
}


////////////////////////////////////////////////////////////////
RTTI::~RTTI() throw()
{
}


////////////////////////////////////////////////////////////////
void RTTI::add_vptr_bit_offset(off_t bitOffset)
{
    bitOffset_ += bitOffset;
    offset_ = bitOffset_ / Platform::byte_size;
}


////////////////////////////////////////////////////////////////
RefPtr<SharedString> RTTI::parse_for_type(SharedString* str)
{
    RefPtr<SharedString> type;
    if (str)
    {
    #ifdef DEBUG
        clog << __func__ << ": " << str << endl;
    #endif
        // GCC -- reverse search for '::'
        const char* v = str->c_str();
        const char* p = v + str->length();

        for (; p >= v; --p)
        {
            if (p[0] == ':' && p[1] == ':')
            {
                break;
            }
        }
        if (p < v)
        {
            p = strstr(v, " virtual table");
        }
        if (p > v)
        {
            // reverse search for ' ', for "non-virtual thunk Blah::"
            // strip everything up to Blah; need to be careful though,
            // we might also see "Klass<Traits<Blah> >::"
            for (const char* r = p - 1; r >= v; --r)
            {
                if (r[0] == ' ' && r[1] != '>')
                {
                    v = r + 1;
                    break;
                }
            }
            type = SharedStringImpl::create(v, p);
            // clog << __func__ << ": " << type.get() << endl;
        }

        if (const char* x = strstr(str->c_str(), " type_info"))
        {
            type = SharedStringImpl::create(str->c_str(), (x - str->c_str()));
            // clog << __func__ << ": " << type.get() << endl;
        }
        // GCC 3.2.2
        else if (strncmp(str->c_str(), "vtable for ", 11) == 0)
        {
            type = SharedStringImpl::create(str->c_str() + 11);
    #ifdef DEBUG
            clog << __func__ << ": " << type.get() << endl;
    #endif
        }
        // GCC 4.0.0
        else if (strncmp(str->c_str(), "typeinfo for ", 13) == 0)
        {
            type = SharedStringImpl::create(str->c_str() + 13);
        }
        // GCC 4.1.0
        else if (strncmp(str->c_str(), "VTT for ", 8) == 0)
        {
            type = SharedStringImpl::create(str->c_str() + 8);
        }
    }
    return type;
}


////////////////////////////////////////////////////////////////
static RefPtr<SharedString>
scan_vtable(const Thread& thread, addr_t addr, size_t nEntries)
{
    RefPtr<SharedString> name; // function name

    thread_read(thread, addr, addr);
    RefPtr<SymbolMap> symbols = CHKPTR(thread.symbols());

    size_t step = sizeof (word_t);

    if ((__WORDSIZE > 4) && thread.is_32_bit())
    {
        step = sizeof(int32_t);
    }
    for (size_t i = 0; i != nEntries; ++i, addr += step)
    {
        RefPtr<Symbol> sym = symbols->lookup_symbol(addr);
        if (!sym)
        {
            dbgout(Log::ALWAYS) << __func__ << ": symbol not found "
                                << hex << addr << dec << endl;
            continue;
        }
        if (sym->offset())
        {
            continue;
        }
        name = RTTI::parse_for_type(sym->demangled_name(false));
        if (name)
        {
            IF_DEBUG_RTTI(
                clog << __func__ << " [" << i << "]=";
                clog << sym->demangled_name(false) << endl;
            )
            break;
        }
    }
    return name;
}


////////////////////////////////////////////////////////////////
RefPtr<SharedString> RTTI::type_name(DebugSymbol& sym)
{
    assert(sym.thread());
    assert(sym.thread()->symbols());

    if (!typename_)
    {
        assert((bitOffset_ % byte_size) == 0);
        IF_DEBUG_RTTI(
            clog << endl << __func__ << ": " << sym.name();
            clog << " type=" << sym.type()->name() << endl;
            clog << " sym.addr()=" << (void*)sym.addr() << endl;
        )
        const addr_t addr = sym.addr() + vptr_offset();

        RefPtr<Thread> thread = CHKPTR(sym.thread());

        if (!always_scan_vtables()) try
        {
            addr_t vptr = 0;
            // read the vptr
            thread_read(*thread, addr, vptr);
            IF_DEBUG_RTTI(
                clog << __func__ << ": vptr=" << hex << vptr << dec << endl;
            )
            if (vptr)
            {
                // read address of typeinfo
                thread_read(*thread, vptr - sizeof(addr_t), vptr);
            }
            IF_DEBUG_RTTI(
                clog << __func__ << ": typeinfo=" << (void*) vptr << endl;
            )
            // read the ptr to typename (skip the typeinfo's vptr)
            thread_read(*thread, vptr + sizeof(addr_t), vptr);

            char name[8192] = { 0 };
            read_string(thread.get(), vptr, name);

            if (name[0])
            {
                IF_DEBUG_RTTI(clog << __func__ << ": typename=" << name << endl);

                // assume CXX V3 ABI mangling
                string mangled = string("_Z") + name;
                size_t len = mangled.size();

                if (char* tmp = unmangle(mangled.c_str(), &len, 0, 0))
                {
                    IF_DEBUG_RTTI(clog << __func__ << ": Typename=" << tmp << endl);
                    return SharedStringImpl::take_ownership(tmp);
                }
            }
        }
        catch (...)
        {
            // quietly eat all errors, and go with plan B...
        }

        // plan B: read the pointers to the virtual functions and
        // attempt to infer the class' name from the func names
        typename_ = scan_vtable(*thread, addr, 20);
    }
    return typename_;
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
