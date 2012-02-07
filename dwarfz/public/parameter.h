#ifndef PARAM_H__4CD0F6D3_DF69_41A9_9A2A_E4C4CE967B80
#define PARAM_H__4CD0F6D3_DF69_41A9_9A2A_E4C4CE967B80
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
//
#include <dwarf.h>
#include "child.h"
#include "datum.h"

namespace Dwarf
{
    template<typename T> class IterationTraits;

    class Function;

    /**
     * Wraps a Dwarf_Die of type DW_TAG_formal_parameter
     */
    CLASS Parameter : public Datum
    {
    public:
        enum { TAG = DW_TAG_formal_parameter };

        // friend class IterationTraits<Parameter>;

        Parameter(Dwarf_Debug dbg, Dwarf_Die die) : Datum(dbg, die)
        { }
    };

    template<> struct Traits<Parameter>
    {
        struct parent_type { /* empty */ };
    };

    template<typename T = Function>
    CLASS ParameterT : public Child<T>, public Parameter
    {
    public:
        // friend class IterationTraits<ParameterT>;

        virtual ~ParameterT() throw() { }

        ParameterT(Dwarf_Debug dbg, Dwarf_Die die)
            : Parameter(dbg, die) { }
    };
}
#endif // PARAM_H__4CD0F6D3_DF69_41A9_9A2A_E4C4CE967B80
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
