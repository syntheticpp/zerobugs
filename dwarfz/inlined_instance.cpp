// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
// $Id: inlined_instance.cpp 713 2010-10-16 07:10:27Z root $
//
#include <iostream>
#include "public/function.h"
#include "public/variable.h"
#include "private/generic_attr.h"
#include "inlined_instance.h"
#include "utils.h"

using namespace std;
using namespace boost;
using namespace Dwarf;


InlinedInstance::InlinedInstance(Dwarf_Debug dbg, Dwarf_Die die)
    : Block(dbg, die)
{
}


boost::shared_ptr<Dwarf::Function> InlinedInstance::function() const
{
    // use DW_AT_abstract origin to get the inlined function die
    boost::shared_ptr<Dwarf::Function> fun(
        shared_dynamic_cast<Function>(check_indirect(false)));
    if (fun)
    {
     /* clog << fun->name() << ": " << hex << fun->low_pc()
             << "-" << fun->high_pc() << dec << endl; */

        if ((fun->low_pc() == 0) && (fun->high_pc() == 0))
        {
            fun->set_range(low_pc(), high_pc());
        }
    }
    return fun;
}


List<Parameter> InlinedInstance::params() const
{
#ifdef DEBUG
    if (boost::shared_ptr<Function> fun = function())
    {
        clog << __func__ << ": " << fun->name() << endl;
    }
#endif
    return List<ParameterT<InlinedInstance> >(dbg(), die());
}


List<VariableT<InlinedInstance> > InlinedInstance::variables() const
{
#ifdef DEBUG
    if (boost::shared_ptr<Function> fun = function())
    {
        clog << __func__ << ": " << fun->name() << endl;
    }
#endif
    return List<VariableT<InlinedInstance> >(dbg(), die());
}


size_t InlinedInstance::call_file() const
{
    size_t file = 0;
    if (Utils::has_attr(dbg(), die(), DW_AT_call_file))
    {
        GenericAttr<DW_AT_call_file, size_t> attr(dbg(), die());
        file = attr.value();
    }
    return file;
}


size_t InlinedInstance::call_line() const
{
    size_t line = 0;
    if (Utils::has_attr(dbg(), die(), DW_AT_call_line))
    {
        GenericAttr<DW_AT_call_line, size_t> attr(dbg(), die());
        line = attr.value();
    }
    return line;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
