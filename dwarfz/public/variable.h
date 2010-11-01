#ifndef VARIABLE_H__E546BD4D_24E0_48B4_826B_805F65342146
#define VARIABLE_H__E546BD4D_24E0_48B4_826B_805F65342146
//
// $Id: variable.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <vector>
#include <boost/shared_ptr.hpp>
#include <dwarf.h>
#include "child.h"
#include "datum.h"
#include "interface.h"

namespace Dwarf
{
    template<typename T> class IterationTraits;

    /**
     * Wrapper for Dwarf_Die of type DW_TAG_variable
     */
    CLASS Variable : public Datum
    {
    public:
        enum { TAG = DW_TAG_variable };

        friend class IterationTraits<Variable>;

        virtual ~Variable() throw();

        Variable(Dwarf_Debug dbg, Dwarf_Die die);
    };


    template<typename T>
    CLASS VariableT : public Variable, public Child<T>
    {
        friend class IterationTraits<Variable>;
        friend class IterationTraits<VariableT>;

        VariableT (Dwarf_Debug dbg, Dwarf_Die die) : Variable(dbg, die)
        { }

    public:
        ~VariableT() throw() { }
    };

    typedef std::vector<boost::shared_ptr<Variable> > VarList;
}
#endif // VARIABLE_H__E546BD4D_24E0_48B4_826B_805F65342146
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
