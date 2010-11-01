// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: flags-i386.cpp 714 2010-10-17 10:03:52Z root $
//
#include "flags-i386.h"
#if DEBUG
 #include <iostream>
#endif

using namespace std;


struct flag
{
    const char* name;
    reg_t       mask;
    size_t      shift;
    const char* desc;
};

static const flag eflags[] = {
    { "CF", 1UL << 0,  0, "Carry flag" },
    { "PF", 1UL << 2,  2, "Parity flag" },
    { "AF", 1UL << 4,  4, "Auxiliary carry flag" },
    { "ZF", 1UL << 6,  6, "Zero flag" },
    { "SF", 1UL << 7,  7, "Sign flag" },
    { "TF", 1UL << 8,  8, "Trap flag" },
    { "IF", 1UL << 9,  9, "Interrupt enable flag" },
    { "DF", 1UL << 10, 10, "Direction flag" },
    { "OF", 1UL << 11, 11, "Overflow flag" },
    { "IOPL", 3 << 12, 12, "I/O Privilege level" },
    { "NT", 1UL << 14, 14, "Nested task flag" },
    { "RF", 1UL << 16, 16, "Resume flag" },
    { "VM", 1UL << 17, 17, "Virtual 8086 mode" },
    { "AC", 1UL << 18, 18, "Alignment check flag" },
    { "VIF",1UL << 19, 19, "Virtual interrupt flag" },
    { "VIP",1UL << 20, 20, "Virtual interrupt pendingflag" },
    { "ID", 1UL << 21, 21, "ID flag" },
};

static const size_t nflags = sizeof(eflags) / sizeof(eflags[0]);

size_t Flags386::enum_fields(
    EnumCallback3<const char*, reg_t, reg_t>* callback) const
{
    if (callback)
    {
        for (size_t n = 0; n != nflags; ++n)
        {
            const reg_t mask = eflags[n].mask;
            const reg_t m = value_ & mask;

            const int value = (m >> eflags[n].shift);

            callback->notify(eflags[n].name, value, mask);
        }
    }
    return nflags;
}


bool Flags386::set_value(const char* value, const char* name)
{
    bool result = false;
    if (!name)
    {
        result = Reg<reg_t>::set_value(value, name);
    }
    else
    {
        for (size_t i = 0; i != nflags; ++i)
        {
            if (strcmp(eflags[i].name, name) == 0)
            {
                const reg_t mask = eflags[i].mask;
                reg_t val = strtoul(value, 0, 0);
                val <<= eflags[i].shift;

                val |= (value_ & ~mask);
                result = commit(val);
            #if DEBUG
                clog << __func__ << "(" << value << ",";
                clog << name << ")=" << result << endl;
            #endif
                break;
            }
        }
    }
    return result;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
