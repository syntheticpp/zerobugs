#ifndef VAR_H__E45E60E8_679F_465E_9DAE_594175807950
#define VAR_H__E45E60E8_679F_465E_9DAE_594175807950
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

#include "wrapper.h"

namespace Dwarf
{
/* specialized traits for Dwarf_Var */
template<> struct WrapperTraits<Dwarf_Var>
{
    static int name(Dwarf_Var var, char** name, Dwarf_Error* err)
    { return dwarf_varname(var, name, err); }

    static void dealloc(Dwarf_Debug dbg, Dwarf_Var var)
    { dwarf_dealloc(dbg, var, DW_DLA_VAR); }

    static int die_offset(Dwarf_Var var, Dwarf_Off* off, Dwarf_Error* err)
    { return dwarf_var_die_offset(var, off, err); }

    static int cu_offset(Dwarf_Var var, Dwarf_Off* off, Dwarf_Error* err)
    { return dwarf_var_cu_offset(var, off, err); }

    static int objects(
        Dwarf_Debug dbg,
        Dwarf_Var** var,
        Dwarf_Signed* cnt,
        Dwarf_Error* err)
    {
        return dwarf_get_vars(dbg, var, cnt, err);
    }
};

typedef Wrapper<Dwarf_Var> Var;
}
#endif // VAR_H__E45E60E8_679F_465E_9DAE_594175807950
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
