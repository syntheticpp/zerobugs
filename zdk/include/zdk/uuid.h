#ifndef UUID_H__CF525794_EF1F_4928_8E57_A67FB48263A6
#define UUID_H__CF525794_EF1F_4928_8E57_A67FB48263A6
//
// $Id: uuid.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <assert.h>
#include <string.h>
#include <iosfwd>

#include <boost/cstdint.hpp>
#include "zdk/export.h"

#ifdef __GNUC__
 struct __attribute__((packed)) ZDK_UUID
#elif _MSC_VER
#pragma pack(push, 1)
 struct ZDK_UUID
#else
 // error
#endif
 {
    boost::uint32_t data1;
    boost::uint16_t data2;
    boost::uint16_t data3;
    boost::uint16_t data4;
    boost::uint8_t  data5[6];
 };
#if _MSC_VER
 #pragma pack(pop)
#endif

// use pointers in interfaces
typedef const ZDK_UUID* uuidref_t;


extern "C" ZDK_UUID* new_uuid_from_string(const char*);

extern "C" ZDK_UUID uuid_from_string(const char*);
extern "C" void uuid_to_string(uuidref_t, char(&)[37]);

int inline uuid_compare(uuidref_t lhs, uuidref_t rhs)
{
    return memcmp(lhs, rhs, sizeof(ZDK_UUID));
}

bool inline operator<(const ZDK_UUID& lhs, const ZDK_UUID& rhs)
{
    return uuid_compare(&lhs, &rhs) < 0;
}

bool inline ZDK_LOCAL uuid_equal(uuidref_t lhs, uuidref_t rhs)
{
    if (lhs == rhs)
        return true;
    return uuid_compare(lhs, rhs) == 0;
}

bool inline operator==(const ZDK_UUID& lhs, const ZDK_UUID& rhs)
{
    return uuid_equal(&lhs, &rhs);
}


std::ostream& operator<<(std::ostream&, const ZDK_UUID& uuid);

#endif // UUID_H__CF525794_EF1F_4928_8E57_A67FB48263A6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
