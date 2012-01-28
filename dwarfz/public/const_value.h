#ifndef CONST_VALUE_H__3408573E_89EA_4B64_A393_ED1B231DB8C8
#define CONST_VALUE_H__3408573E_89EA_4B64_A393_ED1B231DB8C8
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

#include <libdwarf.h>
#include <vector>
#include "interface.h"

namespace Dwarf
{
    CLASS ConstValue
    {
    public:
        typedef std::vector<char> Buffer;

        explicit ConstValue(Dwarf_Unsigned);
        explicit ConstValue(const Buffer&);

        size_t size() const { return data_.size(); }

        const void* data() const { return &data_[0]; }

    private:
        Buffer data_;
    };
};
#endif // CONST_VALUE_H__3408573E_89EA_4B64_A393_ED1B231DB8C8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
