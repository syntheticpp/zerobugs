#ifndef UTIL_H__C0CB51FF_22C2_4B8B_8BE8_B8D24C88B2BA
#define UTIL_H__C0CB51FF_22C2_4B8B_8BE8_B8D24C88B2BA
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

#include "zdk/platform.h"
#include "zdk/shared_string.h"
#include "zdk/type_system.h"
#include "typez/public/debug_symbol.h"
#include "typez/public/value.h"

using Platform::addr_t;
class Thread;


SharedString* null_ptr();



template<typename T, size_t MAXLEN>
ZDK_LOCAL inline void
read_string(Thread* thread, addr_t addr, T(& buf)[MAXLEN], size_t maxlen = MAXLEN)
{
    memset(buf, 0, sizeof buf);

    for (size_t count = 0; ;++count)
    {
        if ((maxlen >= MAXLEN) && (count + 4 >= MAXLEN))
        {
            buf[count++] = T('.');
            buf[count++] = T('.');
            buf[count++] = T('.');
            buf[count] = T();
            break;
        }
        else if (count >= maxlen)
        {
            break;
        }

        Value<T> tmp;
        T c = tmp.read(thread, addr);
        if ((buf[count] = c) != T())
        {
            addr += sizeof(T);
        }
        else
        {
            break;
        }
    }
}


ZDK_LOCAL inline void
emit_macro(Thread& thread,
           DebugSymbolEvents& events,
           const std::string& name,
           const std::string& value)
{
    TypeSystem& types = interface_cast<TypeSystem&>(thread);

    if (RefPtr<DataType> macType = types.get_macro_type())
    {
        // create a constant symbol
        RefPtr<DebugSymbolImpl> sym =
            DebugSymbolImpl::create(
                thread,
                *macType,
                value,
                types.get_string(name.c_str(), name.size()));

        events.notify(sym.get());
    }
}
#endif // UTIL_H__C0CB51FF_22C2_4B8B_8BE8_B8D24C88B2BA
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
