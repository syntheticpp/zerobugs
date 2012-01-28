#ifndef CPU_H__8C16D105_4CD3_4FDB_B281_868AE7B6FDF1
#define CPU_H__8C16D105_4CD3_4FDB_B281_868AE7B6FDF1
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

#include <boost/static_assert.hpp>
#include "zdk/zobject_impl.h"
#include "generic/empty.h"


/**
 * Initializes all struct members to zero;
 * important assumption: R is a C POD struct -- this
 * should be okay, since the unix API headers are in C
 */
template<typename R>
struct RegStruct : public R
{
    RegStruct()
    {
        BOOST_STATIC_ASSERT(sizeof(*this) == sizeof(R));
        memset(this, 0, sizeof(*this));
    }

    explicit RegStruct(const R& other) : R(other) { }
};


/**
 * Wrapper for architecture-dependent user_regs struct
 */
template<typename R = Empty>
struct Regs : public ZObjectImpl<>, public RegStruct<R>
{
DECLARE_UUID("00d7124e-692b-4562-bc83-5d37b8c1d305")
BEGIN_INTERFACE_MAP(Regs)
    INTERFACE_ENTRY(Regs)
END_INTERFACE_MAP()

    explicit Regs(const R& other) : RegStruct<R>(other) { }

    Regs() { }
};


/**
 * Wrapper for architecture-dependent fpu_user_regs struct
 */
template<typename R = Empty>
struct FpuRegs : public ZObjectImpl<>, public RegStruct<R>
{
DECLARE_UUID("a9da2910-6895-4373-9934-251acd895cbf")
BEGIN_INTERFACE_MAP(FpuRegs)
    INTERFACE_ENTRY(FpuRegs)
END_INTERFACE_MAP()

    explicit FpuRegs(const R& other) : RegStruct<R>(other) { }

    FpuRegs() { }
};



#endif // CPU_H__8C16D105_4CD3_4FDB_B281_868AE7B6FDF1
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
