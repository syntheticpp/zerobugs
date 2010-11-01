#ifndef CONST_VALUE_H__D247FABC_08C0_437B_83B1_7F72F61498B8
#define CONST_VALUE_H__D247FABC_08C0_437B_83B1_7F72F61498B8
//
// $Id: const_value.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/ref_ptr.h"

class DataType;
class DebugSymbolImpl;
class SharedString;
class Thread;


namespace Dwarf
{
    class Datum;

    /**
     * @return a debug symbol that corresponds to the
     * DW_AT_const_value attribute of the given datum, or NULL
     */
    RefPtr<DebugSymbolImpl>
    const_value(Thread&, const Datum&, DataType&, SharedString*);
}

#endif // CONST_VALUE_H__D247FABC_08C0_437B_83B1_7F72F61498B8
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
