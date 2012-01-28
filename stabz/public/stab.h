#ifndef STAB_H__44B69FCC_B848_4C41_AFA7_759A54452CF6
#define STAB_H__44B69FCC_B848_4C41_AFA7_759A54452CF6
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
#include "zdk/platform.h"

namespace Stab
{
/*  Entry in the .stabs debug info section.

    @li    4 byte string table index
    @li    1 byte stab type
    @li    1 byte stab other field
    @li    2 byte stab desc field
    @li    4 byte stab value

    @note still 12-byte on x86_64
 */

struct Entry
{
    uint32_t strindex_;
    uint8_t  type_;
    uint8_t  other_;
    uint16_t desc_;
    uint32_t value_;
};

BOOST_STATIC_ASSERT(sizeof(Entry) == 12);

class stab_t : private Entry
{
public:
    unsigned long strindex() const { return strindex_; }
    uint8_t type() const { return type_; }
    uint8_t other() const { return other_; }
    uint16_t desc() const { return desc_; }
    unsigned long value() const { return (int)value_; }
};

} // namespace Stab
#endif // STAB_H__44B69FCC_B848_4C41_AFA7_759A54452CF6
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
